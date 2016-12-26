/*
 * summary_summary_dstack.cpp
 *
 *  Created on: Mar 17, 2015
 *      Author: Julian Kranz
 */

#include <algorithm>
#include <assert.h>
#include <bjutil/printer.h>
#include <cppgdsl/rreil/rreil.h>
#include <experimental/optional>
#include <functional>
#include <iterator>
#include <summy/analysis/domains/api/api.h>
#include <summy/analysis/domains/mempath.h>
#include <summy/analysis/domains/numeric/als_state.h>
#include <summy/analysis/domains/numeric/equality_state.h>
#include <summy/analysis/domains/numeric/vsd_state.h>
#include <summy/analysis/domains/summary_application.h>
#include <summy/analysis/domains/summary_dstack.h>
#include <summy/analysis/static_memory.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <summy/cfg/node/address_node.h>
#include <summy/cfg/node/node.h>
#include <summy/cfg/node/node_visitor.h>
#include <summy/cfg/observer.h>
#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/memory_id.h>
#include <summy/rreil/id/sm_id.h>
#include <summy/value_set/value_set_visitor.h>
#include <summy/value_set/vs_finite.h>

using cfg::address_node;
using cfg::cond_edge;
using cfg::call_edge;
using cfg::decoding_state;
using cfg::edge_visitor;
using cfg::node;
using cfg::node_visitor;
using cfg::stmt_edge;
using summy::value_set_visitor;
using summy::vs_finite;

using namespace analysis;
using namespace std;
using namespace gdsl::rreil;
using namespace analysis::value_sets;
using namespace api;
using namespace summy::rreil;
using namespace summy;
using namespace std::experimental;

std::experimental::optional<summary_t> analysis::summary_dstack::get_stub(
  void *address, size_t node) {
  //  symbol symb;
  //  bool success;
  //  tie(success, symb) = sm->lookup(address);
  //  cout << symb.symbol_name << endl;
  //  cout << hex << address << dec << endl;

  auto functions = sm->functions();
  if(!functions) return nullopt;

  auto f_it = functions.value().get().find((size_t)address);
  if(f_it != functions.value().get().end()) {
    string &name = f_it->second.name;

    string _malloc = "malloc";
    if(name.compare(0, _malloc.length(), _malloc) == 0) {
      return stubs.allocator(node, 0);
    }

    string _new = "_Znwm";
    if(name.compare(0, _new.length(), _new) == 0) {
      return stubs.allocator(node, 0);
    }

    string _new_array = "_Znam";
    if(name.compare(0, _new_array.length(), _new_array) == 0) {
      return stubs.allocator(node, 0);
    }

    string longjmp = "longjmp";
    if(name.compare(0, longjmp.length(), longjmp) == 0) {
      return stubs.bottomifier();
    }

    if(f_it->second.lt == DYNAMIC) {
      if(warnings) cout << "Warning: Ignoring call to " << name << "." << endl;
      return stubs.no_effect();
    }
  }
  return nullopt;
}

std::set<size_t> analysis::summary_dstack::get_callers(std::shared_ptr<global_state> state) {
  vs_shared_t f_addrs = state->get_f_addr();
  callers_t callers;
  value_set_visitor vsv;
  vsv._([&](vs_finite const *vf) {
    for(size_t f_addr : vf->get_elements()) {
      size_t head_id = this->function_desc_map.at((void *)f_addr).head_id;
      for(size_t parent : cfg->in_edges(head_id)) {
        cfg::edge const *e = cfg->out_edge_payloads(parent)->at(head_id);
        edge_visitor ev;
        ev._([&](cfg::call_edge const *) { callers.insert(parent); });
        e->accept(ev);
      }
    }
  });
  f_addrs->accept(vsv);
  return callers;
}

std::set<size_t> analysis::summary_dstack::get_function_heads(std::shared_ptr<global_state> state) {
  vs_shared_t f_addrs = state->get_f_addr();
  callers_t heads;
  value_set_visitor vsv;
  vsv._([&](vs_finite const *vf) {
    for(size_t f_addr : vf->get_elements()) {
      size_t head_id = this->function_desc_map.at((void *)f_addr).head_id;
      heads.insert(head_id);
    }
  });
  f_addrs->accept(vsv);
  return heads;
}

set<void *> analysis::summary_dstack::unpack_f_addrs(summy::vs_shared_t f_addr) {
  set<void *> addresses;
  value_set_visitor vsv(true);
  vsv._([&](vs_finite const *vsf) {
    vs_finite::elements_t const &elems = vsf->get_elements();
    for(auto address : elems) {
      void *address_ptr = (void *)address;
      addresses.insert(address_ptr);
    }
  });
  f_addr->accept(vsv);
  return addresses;
}

void analysis::summary_dstack::propagate_reqs(std::set<mempath> field_reqs_new, void *f_addr) {
  auto &fd = function_desc_map.at(f_addr);
  if(!includes(
       fd.field_reqs.begin(), fd.field_reqs.end(), field_reqs_new.begin(), field_reqs_new.end())) {
    fd.field_reqs.insert(field_reqs_new.begin(), field_reqs_new.end());
    _dirty_nodes.insert(fd.head_id);
    //    cout << "Added dirty node because of reqs..." << endl;
  }
}

void analysis::summary_dstack::add_constraint(size_t from, size_t to, const ::cfg::edge *e) {
  //  cout << cfg->out_edge_payloads(from)->size() << endl;
  //    cout << "Adding constraint from " << from << " to " << to << endl;
  //    assert(from != 99 || to != 70);
  node_visitor nv;
  nv._([&](address_node *av) {
    ref(from, nullopt);
    ref(to, nullopt);
  });
  cfg->get_node_payload(to)->accept(nv);

  function<shared_ptr<global_state>()> transfer_f = [=]() { return get_sub(from); };
  auto for_mutable = [&](function<void(summary_memory_state *)> cb) {
    transfer_f = [=]() {
      shared_ptr<global_state> state_c = get_sub(from);
      summary_memory_state *mstate_new = state_c->get_mstate()->copy();
      cb(mstate_new);
      shared_ptr<global_state> global_new =
        shared_ptr<global_state>(new global_state(mstate_new, state_c->get_f_addr()));
      return global_new;
    };
  };
  auto for_update = [&](auto *update) {
    for_mutable([=](summary_memory_state *state_new) { state_new->update(update); });
  };
  edge_visitor ev;

  //  statement *_stmt =  NULL;
  //  bool _cond = false;
  //  bool _call = false;

  ev._([&](const stmt_edge *edge) {
    statement *stmt = edge->get_stmt();
    //    _stmt = stmt;
    statement_visitor v;
    v._([&](assign const *a) { for_update(a); });
    v._([&](load const *l) { for_update(l); });
    v._([&](store const *s) { for_update(s); });
    v._([&](branch const *b) {
      if(node_targets.find(from) == node_targets.end()) node_targets[from] = set<size_t>();

      ref(from, nullopt);
      ref(to, nullopt);
      switch(b->get_hint()) {
        case gdsl::rreil::BRANCH_HINT_CALL: {
          transfer_f = [=]() {
            shared_ptr<global_state> state_c = get_sub(from);
            summary_memory_state *mstate = state_c->get_mstate();

            for(auto edge_mapping : *cfg->out_edge_payloads(to))
              _dirty_nodes.insert(edge_mapping.first);

            //            cout << *mstate << endl;

            set<void *> f_addrs = unpack_f_addrs(state_c->get_f_addr());
            assert(f_addrs.size() > 0);
            size_t current_min_calls_sz = 0;
            for(auto f_addr : f_addrs) {
              auto f_min_calls_sz = function_desc_map.at(f_addr).min_calls_sz;
              if(f_min_calls_sz > current_min_calls_sz) current_min_calls_sz = f_min_calls_sz;
            }

            optional<shared_ptr<summary_memory_state>> summary;
            bool directly_recursive = false;

            /*
             * Todo: This is an expensive hack to recognize recursion
             */
            set<void *> callers_addrs_trans = f_addrs;
            vector<size_t> callers_rest;
            callers_t caller_callers = get_callers(state_c);
            callers_rest.insert(callers_rest.begin(), caller_callers.begin(), caller_callers.end());
            while(!callers_rest.empty()) {
              size_t caller = callers_rest.back();
              callers_rest.pop_back();
              set<void *> caller_f_addrs = unpack_f_addrs(get_sub(caller)->get_f_addr());

              set<void *> intersection;
              set_intersection(caller_f_addrs.begin(), caller_f_addrs.end(),
                callers_addrs_trans.begin(), callers_addrs_trans.end(),
                inserter(intersection, intersection.begin()));
              if(intersection.size() > 0) {
                continue;
              }

              callers_addrs_trans.insert(caller_f_addrs.begin(), caller_f_addrs.end());
              callers_t caller_caller_callers = get_callers(get_sub(caller));
              callers_rest.insert(
                callers_rest.begin(), caller_caller_callers.begin(), caller_caller_callers.end());
            }

            ptr_set_t callee_aliases = mstate->queryAls(&b->get_target());
            id_set_t field_req_ids_new;
            //                        cout << *b->get_target() << endl;
            //                        cout << callee_aliases << endl;
            for(auto ptr : callee_aliases) {
              summy::rreil::id_visitor idv;
              bool is_valid_code_address = false;
              void *text_address;
              idv._([&](sm_id const *sid) {
                if(sid->get_symbol() == ".text" || sid->get_symbol() == ".plt") {
                  is_valid_code_address = true;
                  text_address = sid->get_address();
                }
              });
              idv._([&](ptr_memory_id const *) {
                //                cout << "There seems to be an unkown function pointer :/" << endl;
                field_req_ids_new.insert(ptr.id);
              });
              ptr.id->accept(idv);
              if(!is_valid_code_address) continue;
              value_set_visitor vsv;
              vsv._([&](vs_finite const *vsf) {
                //                cout << "-----" << endl;
                for(int64_t offset : vsf->get_elements()) {
                  void *address = (char *)text_address + offset;
                  node_targets[from].insert((size_t)address);
                  //                  cout << "Looking up summary for " << hex << address << dec <<
                  //                  endl;
                  if(callers_addrs_trans.find(address) != callers_addrs_trans.end()) {
                    //                    cout << "Warning: Ignoring recursive call." << endl;
                    continue;
                  }
                  auto add_summary = [&](summary_memory_state *next) {
                    if(summary)
                      summary = shared_ptr<summary_memory_state>(summary.value()->join(next, to));
                    else
                      summary = shared_ptr<summary_memory_state>(next->copy());
                  };
                  optional<summary_t> stub = get_stub(address, from);
                  if(stub) {
                    add_summary(stub.value().get());
                  } else {
                    auto fd_it = function_desc_map.find(address);
                    if(fd_it != function_desc_map.end()) {
                      auto &f_desc = fd_it->second;
                      for(size_t s_node : f_desc.summary_nodes) {
                        add_summary(get_sub(s_node)->get_mstate());
                        if(state_c->get_f_addr() == get_sub(s_node)->get_f_addr())
                          directly_recursive = true;
                      }

                      //                    cout << address << endl;
                      //                    cout << "=====================================" << endl;
                      //                    if(this->state.size() > 91)
                      //            cout << *this->state[91]->get_mstate() << endl;
                      //                    cout << "==================================+++" << endl;
                      //                    cout << *f_desc.summary.get() << endl;

                      f_desc.min_calls_sz = std::max(f_desc.min_calls_sz, current_min_calls_sz);

                      size_t head_id = fd_it->second.head_id;
                      cfg::in_edges_t const &head_in = cfg->in_edges(head_id);
                      if(head_in.find(to) == head_in.end()) {
                        // cout << "New edge from " << to << " to " << head_id << endl;
                        cfg->update_edge(to, head_id, new call_edge(true));
                      }
                    } else {
                      size_t an_id = cfg->create_node([&](size_t id) {
                        return new address_node(id, (size_t)address, cfg::DECODABLE);
                      });
                      function_desc_map.insert(
                        make_pair(address, function_desc(current_min_calls_sz + 1, an_id)));
                      // cout << "New edge from " << to << " to " << an_id << endl;
                      cfg->update_edge(to, an_id, new call_edge(true));
                    }
                  }
                }
                //                cout << "-----" << endl;
              });
              ptr.offset->accept(vsv);
              //              cout << ptr << endl;
            }
            cfg->commit_updates();

            set<mempath> field_reqs_new = mempath::from_aliases(field_req_ids_new, mstate);
            //            propagate_reqs(f_addr, mps);
            for(auto f_addr : f_addrs)
              propagate_reqs(field_reqs_new, f_addr);

            //            cout << "Need to apply the following summary: " << endl;
            //            cout << *summary << endl;

            shared_ptr<summary_memory_state> bottom =
              shared_ptr<summary_memory_state>(sms_bottom());
            if(directly_recursive && summary)
              /*
               * Directly recursive call => We have to rename variables!
               */
              summary.value()->rename();
            //                        if(summary)
            //                          cout << *summary.value();
            //                        if(summary)
            //                          cout << "We have a summary!" << endl;
            //                        else
            //                          cout << "We don't have a summary :-(." << endl;
            //            else
            //              cout << "We don't have a summary :-/" << endl;

            //            summary_memory_state *summarized = summary ? apply_summary(mstate,
            //            summary.value().get()) :
            //            bottom->copy();

            summary_memory_state *summarized =
              summary ? summary_application(mstate, summary.value().get()).apply_summary()
                      : mstate->copy();

            //            summary_memory_state *summarized;
            //            if(summary)
            //              summarized = apply_summary(mstate, summary.value().get());
            //            else {
            //              summarized = mstate->copy();
            //              summarized->topify();
            //            }

            //            if(summarized->is_bottom())
            //              cout << "BOTTOM!" << endl;
            //            else
            //              cout << "NOOOO BOTTOM :-/!" << endl;

            //            cout << *summarized << endl;

            return shared_ptr<global_state>(new global_state(summarized, state_c->get_f_addr()));
          };
          break;
        }
        case gdsl::rreil::BRANCH_HINT_RET: {
          transfer_f = [=]() {
            shared_ptr<global_state> state_c = get_sub(from);
            set<void *> f_addrs = unpack_f_addrs(state_c->get_f_addr());
            assert(f_addrs.size() > 0);
            for(auto f_addr : f_addrs) {
              auto fd_it = function_desc_map.find(f_addr);
              //            if(summary_it == summary_map.end())
              //              summary_map[state_c->get_f_addr()] =
              //              shared_ptr<summary_memory_state>(state_c->get_mstate()->copy());
              //            else
              fd_it->second.summary_nodes.insert(to);
              callers_t caller_callers = get_callers(state_c);
              for(size_t caller : caller_callers) {
                //                cout << "Return-propagating from " << to << " / " << hex << f_addr
                //                << dec << " to " << caller << endl;
                assert_dependency(gen_dependency(to, caller));
              }
            }
            return state_c;
          };
          break;
        }
        case gdsl::rreil::BRANCH_HINT_JUMP: {
          transfer_f = [=]() {
            shared_ptr<global_state> state_c = get_sub(from);
            summary_memory_state *mstate = state_c->get_mstate();
            ptr_set_t callee_aliases = mstate->queryAls(&b->get_target());

            //                        cout << *b->get_target() << endl;
            //                        cout << callee_aliases << endl;
            for(auto ptr : callee_aliases) {

              summy::rreil::id_visitor idv;
              bool is_valid_code_address = false;
              void *text_address;
              idv._([&](sm_id const *sid) {
                if(sid->get_symbol() == ".text" || sid->get_symbol() == ".plt") {
                  is_valid_code_address = true;
                  text_address = sid->get_address();
                }
              });
              ptr.id->accept(idv);
              if(!is_valid_code_address) continue;

              /*
               * Todo: Find address nodes globally and reuse them
               */
              set<size_t> child_addresses;
              for(auto edge_it : *cfg->out_edge_payloads(to)) {
                node_visitor nv;
                nv._([&](address_node *an) { child_addresses.insert(an->get_address()); });
                cfg->get_node_payload(edge_it.first)->accept(nv);
              }


              value_set_visitor vsv;
              vsv._([&](vs_finite const *vsf) {
                //                cout << "-----" << endl;
                for(int64_t offset : vsf->get_elements()) {
                  size_t address = (size_t)text_address + offset;
                  node_targets[from].insert(address);
                  if(child_addresses.find(address) == child_addresses.end()) {
                    size_t child_id = cfg->create_node(
                      [&](size_t id) { return new address_node(id, address, cfg::DECODABLE); });
                    cfg->update_edge(to, child_id, new cfg::edge());
                  }
                }
                //                cout << "Adding " << vsf->get_elements().size() << " many
                //                dests..." << endl;
                //                cout << "-----" << endl;
              });
              ptr.offset->accept(vsv);
            }
            cfg->commit_updates();
            return state_c;
          };
          break;
        }
      }
    });
    stmt->accept(v);
  });
  ev._([&](const cond_edge *edge) {
    //    _cond = true;
    for_mutable([=](summary_memory_state *state_new) {
      if(edge->is_positive())
        state_new->assume(edge->get_cond());
      else
        state_new->assume_not(edge->get_cond());
    });
  });
  ev._([&](const call_edge *edge) {
    ref(from, nullopt);
    ref(to, nullopt);
    //    _call = true;
    transfer_f = [=]() {
      shared_ptr<global_state> state_new;
      if(edge->is_target_edge()) {
        node *dest = cfg->get_node_payload(to);
        bool is_addr_node = false;
        void *address;
        node_visitor nv;
        nv._([&](address_node *an) {
          is_addr_node = true;
          address = (void *)an->get_address();
        });
        dest->accept(nv);
        assert(is_addr_node);
        state_new =
          dynamic_pointer_cast<global_state>(start_value(vs_finite::single((int64_t)address)));

        auto &desc = this->function_desc_map.at(address);
        if(desc.field_reqs.size() > 0) {
          auto const &in_edges = cfg->in_edges(from);
          assert(in_edges.size() == 1);
          size_t from_parent = *in_edges.begin();

          //          cout << "This call requires the following fields:" << endl;
          for(auto &f : desc.field_reqs) {
            //            cout << f << endl;

            auto insert = [&](
              size_t ptr) { (this->pointer_props[(size_t)address])[f].insert(ptr); };

            optional<set<mempath>> mempaths_new =
              f.propagate(insert, get_sub(from_parent)->get_mstate(), state_new->get_mstate());
            if(mempaths_new) {
              //              cout << "Propagating..." << endl;
              //              for(auto p : mempaths_new.value())
              //                cout << p << endl;

              set<void *> f_addrs = unpack_f_addrs(get_sub(from_parent)->get_f_addr());
              assert(f_addrs.size() > 0);
              for(auto f_addr : f_addrs)
                propagate_reqs(mempaths_new.value(), f_addr);
            }
          }
        }
      } else {
        state_new = get_sub(from);
        if(!state_new->get_mstate()->is_bottom()) {
          bool needs_decoding = false;
          size_t addr;
          node_visitor nv;
          nv._([&](address_node *av) {
            if(av->get_decs() != cfg::DECODED && av->get_decs() != cfg::DECODABLE) {
              needs_decoding = true;
              addr = av->get_address();
            }
          });
          this->cfg->get_node_payload(to)->accept(nv);

          if(needs_decoding) {
            this->cfg->replace_node_payload(new address_node(to, addr, cfg::DECODABLE));
            /**
             * Todo: The node state is replaced...?
             */
            this->cfg->commit_updates();
          }
        }
      }
      //      this->cfg->commit_updates();
      return state_new;
    };
  });
  e->accept(ev);
  (constraints[to])[from] = [=]() {
    //    if(_stmt != NULL)
    //      cout << *_stmt << endl;
    //    cout << "cond: " << _cond << endl;
    //    cout << "call: " << _call << endl;
    return transfer_f();
  };
}

void analysis::summary_dstack::remove_constraint(size_t from, size_t to) {
  constraints[to].erase(from);
}

dependency analysis::summary_dstack::gen_dependency(size_t from, size_t to) {
  //  cout << "Generating dep. " << from << " to " << to << endl;
  return dependency{from, to};
}

void analysis::summary_dstack::init_state(summy::vs_shared_t f_addr) {
  //  cout << "init_state()" << endl;

  size_t old_size = state.size();
  state.resize(cfg->node_count());
  for(size_t i = old_size; i < cfg->node_count(); i++) {
    if(fixpoint_pending.find(i) != fixpoint_pending.end()) {
      state[i] = dynamic_pointer_cast<global_state>(start_value(f_addr));
      ref(i, nullopt);
    } else
      state[i] = dynamic_pointer_cast<global_state>(bottom());
  }
}

void analysis::summary_dstack::init_state() {
  init_state(value_set::bottom);
}

analysis::summary_dstack::summary_dstack(
  cfg::cfg *cfg, std::shared_ptr<static_memory> sm, bool warnings, std::set<size_t> const &f_starts)
    : fp_analysis(cfg), sm(sm), warnings(warnings), stubs(sm, warnings) {
  init();

  for(auto node_id : f_starts) {
    node *n = cfg->get_node_payload(node_id);
    optional<size_t> addr;
    node_visitor nv;
    nv._([&](address_node *an) { addr = an->get_address(); });
    n->accept(nv);
    function_desc_map.insert(make_pair((void *)addr.value(), function_desc(0, n->get_id())));
    state[n->get_id()]->set_f_addr(vs_finite::single(addr.value()));
  }
}

analysis::summary_dstack::summary_dstack(
  cfg::cfg *cfg, std::shared_ptr<static_memory> sm, bool warnings)
    : fp_analysis(cfg), sm(sm), warnings(warnings), stubs(sm, warnings) {
  init();

  /*
   * Simulate initial call to node zero
   */
  node *n = cfg->get_node_payload(0);
  optional<size_t> addr;
  node_visitor nv;
  nv._([&](address_node *an) { addr = an->get_address(); });
  n->accept(nv);
  function_desc_map.insert(make_pair((void *)addr.value(), function_desc(0, n->get_id())));
  state[n->get_id()]->set_f_addr(vs_finite::single(addr.value()));
}

analysis::summary_dstack::summary_dstack(cfg::cfg *cfg, bool warnings)
    : summary_dstack(cfg, make_shared<static_dummy>(), warnings) {
  init();
}

analysis::summary_dstack::~summary_dstack() {}

summary_memory_state *analysis::summary_dstack::sms_bottom() {
  return sms_bottom(sm, warnings);
}

summary_memory_state *analysis::summary_dstack::sms_top() {
  return sms_top(sm, warnings);
}

summary_memory_state *analysis::summary_dstack::sms_bottom(
  std::shared_ptr<static_memory> sm, bool warnings) {
  return summary_memory_state::bottom(
    sm, warnings, new equality_state(new als_state(vsd_state::bottom(sm))));
}

summary_memory_state *analysis::summary_dstack::sms_top(
  std::shared_ptr<static_memory> sm, bool warnings) {
  summary_memory_state *sms_start = summary_memory_state::start_value(
    sm, warnings, new equality_state(new als_state(vsd_state::top(sm))));
  return sms_start;
}

shared_ptr<domain_state> analysis::summary_dstack::bottom() {
  return shared_ptr<domain_state>(new global_state(sms_bottom(), value_set::bottom));
}

std::shared_ptr<domain_state> analysis::summary_dstack::start_value(vs_shared_t f_addr) {
  return shared_ptr<domain_state>(new global_state(sms_top(), f_addr));
}

shared_ptr<domain_state> analysis::summary_dstack::get(size_t node) {
  //  if(node >= state.size())
  //    return bottom();
  //  assert(erased.find(node) == erased.end());
  return state[node];
}

shared_ptr<global_state> analysis::summary_dstack::get_sub(size_t node) {
  //  if(node >= state.size())
  //    return bottom();
  //  assert(erased.find(node) == erased.end());
  return state[node];
}

void analysis::summary_dstack::update(size_t node, shared_ptr<domain_state> state) {
  this->state[node] = dynamic_pointer_cast<global_state>(state);
  //  erased.erase(node);
  //  this->state[node]->get_mstate()->check_consistency();
}

summary_dstack_result analysis::summary_dstack::result() {
  return summary_dstack_result(state);
}

node_compare_t analysis::summary_dstack::get_fixpoint_node_comparer() {
  return [=](size_t const &a, size_t const &b) -> optional<bool> {
    if(a >= state.size() || b >= state.size()) return a < b;
    shared_ptr<global_state> state_a = this->state[a];
    shared_ptr<global_state> state_b = this->state[b];
    //    cout << state_a->get_f_addr() << " " << state_b->get_f_addr() << endl;

    set<void *> f_addrs_a = unpack_f_addrs(state_a->get_f_addr());

    set<void *> f_addrs_b = unpack_f_addrs(state_b->get_f_addr());

    //    cout << a << " < " << b << " ~~~ " << *state_a->get_f_addr() << " //// " <<
    //    *state_b->get_f_addr() << endl;

    if(f_addrs_a.size() != 0 && f_addrs_b.size() == 0)
      return true;
    else if(f_addrs_a.size() == 0 && f_addrs_b.size() != 0)
      return false;
    else if(f_addrs_a.size() != 0 && f_addrs_b.size() != 0) {
      size_t min_calls_sz_a = 0;
      for(auto f_addr_a : f_addrs_a) {
        size_t min_calls_sz_curr = function_desc_map.at(f_addr_a).min_calls_sz;
        if(min_calls_sz_curr > min_calls_sz_a) min_calls_sz_a = min_calls_sz_curr;
      }
      size_t min_calls_sz_b = 0;
      for(auto f_addr_b : f_addrs_b) {
        size_t min_calls_sz_curr = function_desc_map.at(f_addr_b).min_calls_sz;
        if(min_calls_sz_curr > min_calls_sz_b) min_calls_sz_b = min_calls_sz_curr;
      }
      //      cout << a << " < " << b << ";;; " << min_calls_sz_a << " //// " << min_calls_sz_b <<
      //      endl;
      if(min_calls_sz_a > min_calls_sz_b)
        return true;
      else if(min_calls_sz_a < min_calls_sz_b)
        return false;
      else
        return nullopt;
    } else
      return nullopt;
  };
}

std::set<size_t> analysis::summary_dstack::dirty_nodes() {
  set<size_t> dirty_nodes = this->_dirty_nodes;
  this->_dirty_nodes.clear();
  return dirty_nodes;
}

void analysis::summary_dstack::check_consistency() {
  /*
   * Todo: Mittels einer extra Knotenliste soll sichergestellt werden,
   * dass Felder nicht verschwinden zwischen den Iterationen
   */
}

void analysis::summary_dstack::ref(size_t node, std::experimental::optional<size_t> count) {
  if(!count)
    ref_map[node] = nullopt;
  else {
    auto ref_it = ref_map.find(node);
    if(ref_it == ref_map.end())
      ref_map[node] = count;
    else {
      auto count_current = ref_it->second;
      if(count_current) ref_it->second = count_current.value() + count.value();
    }
  }
}

void analysis::summary_dstack::unref(size_t node) {
  auto ref_it = ref_map.find(node);
  if(ref_it != ref_map.end()) {
    auto count_current = ref_it->second;
    if(count_current) {
      if(count_current.value() > 1)
        ref_it->second = count_current.value() - 1;
      else {
        //        cout << "Bottomifying node " << node << endl;
        state[node] = dynamic_pointer_cast<global_state>(bottom());
        //        erased.insert(node);
        ref_map.erase(node);
      }
    }
  }
}

optional<size_t> analysis::summary_dstack::get_lowest_function_address(size_t node_id) {
  if(this->state.size() <= node_id) return nullopt;
  set<void *> f_addrs = unpack_f_addrs(state[node_id]->get_f_addr());
  if(f_addrs.size() == 0) return nullopt;
  return (size_t)*f_addrs.begin();
}

void analysis::summary_dstack::print_callstack(size_t node_id) {
  //  cout << *state[node_id] << endl;
  if(this->state.size() <= node_id) return;
  set<size_t> callers_current_heads = get_function_heads(state[node_id]);
  set<size_t> callers_addrs_trans;
  auto print_node = [&](size_t node_id) {
    bool handled = false;
    node_visitor nv;
    nv._([&](address_node *an) {
      handled = true;
      cout << "0x" << hex << an->get_address() << dec << "(" << node_id << ")";
      if(an->get_name()) cout << ": " << an->get_name().value();
    });
    cfg->get_node_payload(node_id)->accept(nv);
    if(!handled) cout << "?(" << node_id << ")";
  };
  cout << "----> Call stack for ";
  print_node(node_id);
  cout << endl;
  bool level_first = true;
  while(!callers_current_heads.empty()) {
    if(level_first)
      level_first = false;
    else
      cout << endl;
    bool first = true;
    for(size_t caller : callers_current_heads) {
      if(!first)
        cout << ", ";
      else
        first = false;
      print_node(caller);
    }

    set<size_t> callers_next_heads;
    for(size_t caller : callers_current_heads) {
      set<size_t> callers_next = get_callers(state[caller]);
      for(auto caller_next : callers_next) {
        set<size_t> callers_next_heads_current = get_function_heads(state[caller_next]);
        callers_next_heads.insert(
          callers_next_heads_current.begin(), callers_next_heads_current.end());
      }
    }

    for(auto caller_trans : callers_addrs_trans)
      callers_next_heads.erase(caller_trans);
    callers_addrs_trans.insert(callers_next_heads.begin(), callers_next_heads.end());
    callers_current_heads = callers_next_heads;
  }
  cout << endl;
  cout << "<---- End of call stack" << endl;
}

void analysis::summary_dstack::put(std::ostream &out) {
  for(size_t i = 0; i < state.size(); i++)
    out << "Node " << i << ": " << endl << *state[i] << endl;
}
