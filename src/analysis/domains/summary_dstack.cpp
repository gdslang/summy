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
#include <functional>
#include <iterator>
#include <optional>
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
using cfg::call_edge;
using cfg::cond_edge;
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


std::optional<summary_t> analysis::summary_dstack::get_stub(void *address, size_t node) {
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

    // GDSL alloc
    string _alloc = "alloc";
    if(name.compare(0, _alloc.length(), _alloc) == 0) {
      return stubs.allocator(node, 0);
    }

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

    string printf = "printf";
    if(name.compare(0, printf.length(), printf) == 0) {
      if(warnings) cout << "Ignoring call to " << name << "." << endl;
      return stubs.no_effect();
    }

    string putchar = "putchar";
    if(name.compare(0, putchar.length(), putchar) == 0) {
      if(warnings) cout << "Ignoring call to " << name << "." << endl;
      return stubs.no_effect();
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
    auto ctx_mapping = state[fd.head_id].begin();
    assert(ctx_mapping != state[fd.head_id].end());
    _dirty_nodes.context_deps[ctx_mapping->first].insert(fd.head_id);

    //     cout << "Added dirty node " << fd.head_id << " for address 0x" << std::hex << f_addr <<
    //     std::dec
    //          << " because of reqs..." << endl;
  }
}

std::map<size_t, std::shared_ptr<domain_state>> analysis::summary_dstack::transform(
  size_t from, size_t to, const ::cfg::edge *e, size_t from_ctx) {

  std::map<size_t, std::shared_ptr<domain_state>> result =
    default_context(get_sub(from, from_ctx), from_ctx);
  auto for_mutable = [&](function<void(summary_memory_state *)> cb) {
    shared_ptr<global_state> state_c = get_sub(from, from_ctx);
    summary_memory_state *mstate_new = state_c->get_mstate()->copy();
    cb(mstate_new);
    shared_ptr<global_state> global_new =
      shared_ptr<global_state>(new global_state(mstate_new, state_c->get_f_addr()));
    result = default_context(global_new, from_ctx);
  };
  auto for_update = [&](auto *update) {
    for_mutable([=](summary_memory_state *state_new) { state_new->update(update); });
  };

  edge_visitor ev;

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
          shared_ptr<global_state> state_c = get_sub(from, from_ctx);
          summary_memory_state *mstate = state_c->get_mstate();

          for(auto edge_mapping : *cfg->out_edge_payloads(to))
            _dirty_nodes.context_deps[from_ctx].insert(edge_mapping.first);

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
            set<void *> caller_f_addrs = unpack_f_addrs(get_sub(caller, from_ctx)->get_f_addr());

            set<void *> intersection;
            set_intersection(caller_f_addrs.begin(), caller_f_addrs.end(),
              callers_addrs_trans.begin(), callers_addrs_trans.end(),
              inserter(intersection, intersection.begin()));
            if(intersection.size() > 0) {
              continue;
            }

            callers_addrs_trans.insert(caller_f_addrs.begin(), caller_f_addrs.end());
            callers_t caller_caller_callers = get_callers(get_sub(caller, from_ctx));
            callers_rest.insert(
              callers_rest.begin(), caller_caller_callers.begin(), caller_caller_callers.end());
          }

          ptr_set_t callee_aliases = mstate->queryAls(&b->get_target());
          id_set_t field_req_ids_new;
          //                        cout << *b->get_target() << endl;
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
                      auto tabulate_all = [&]() {
                        std::set<mempath> remaining; // Don't care?!
                        size_t path_construction_errors = 0;
                        std::vector<std::set<mempath_assignment>> assignments_sets =
                          tabulation_keys(
                            remaining, path_construction_errors, f_desc, state_c->get_mstate());
                        for(auto &assignment_set : assignments_sets) {
                          size_t ctx;
                          if(assignment_set.size() == 0)
                            ctx = 0; // No HBs => default context
                          else {
                            auto table_key = tabulation_key(assignment_set);
                            auto ctx_it = f_desc.contexts.find(table_key);
                            if(ctx_it == f_desc.contexts.end())
                              continue; // Todo: What has happened here?
                            ctx = ctx_it->second;
                          }

                          add_summary(get_sub(s_node, ctx)->get_mstate());
                          if(state_c->get_f_addr() == get_sub(s_node, ctx)->get_f_addr())
                            directly_recursive = true;
                        }
                      };

                      auto accumulate_all = [&]() {
                        add_summary(get_sub(s_node, from_ctx)->get_mstate());
                        if(state_c->get_f_addr() == get_sub(s_node, from_ctx)->get_f_addr())
                          directly_recursive = true;
                      };

                      if(tabulation)
                        tabulate_all();
                      else
                        accumulate_all();
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

          if(field_req_ids_new.size() > unique_hbs[from])
            unique_hbs[from] = field_req_ids_new.size();

          set<mempath> field_reqs_new;
          size_t path_construction_errors =
            mempath::from_aliases(field_reqs_new, field_req_ids_new, mstate);
          (this->path_construction_errors[from])[from_ctx] = path_construction_errors;
          //             for(auto &req : field_reqs_new)
          //               std::cout << req << std::endl;
          //            propagate_reqs(f_addr, mps);
          for(auto f_addr : f_addrs)
            propagate_reqs(field_reqs_new, f_addr);

          //            cout << "Need to apply the following summary: " << endl;
          //            cout << *summary << endl;

          shared_ptr<summary_memory_state> bottom = shared_ptr<summary_memory_state>(sms_bottom());
          if(directly_recursive && summary)
            /*
             * Directly recursive call => We have to rename variables!
             */
            summary.value()->rename();


          summary_memory_state *summarized =
            summary ? summary_application(mstate, summary.value().get()).apply_summary()
                    : mstate->copy();

          result = default_context(
            shared_ptr<global_state>(new global_state(summarized, state_c->get_f_addr())),
            from_ctx);
          break;
        }
        case gdsl::rreil::BRANCH_HINT_RET: {
          shared_ptr<global_state> state_c = get_sub(from, from_ctx);
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
          result = default_context(state_c, from_ctx);
          break;
        }
        case gdsl::rreil::BRANCH_HINT_JUMP: {
          shared_ptr<global_state> state_c = get_sub(from, from_ctx);
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
          result = default_context(state_c, from_ctx);
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
    std::map<size_t, shared_ptr<domain_state>> state_map_new;
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
      auto state_new =
        dynamic_pointer_cast<global_state>(start_state(vs_finite::single((int64_t)address)));

      auto &desc = this->function_desc_map.at(address);
      if(desc.field_reqs.size() > 0) {
        auto const &in_edges = cfg->in_edges(from);
        assert(in_edges.size() == 1);
        size_t from_parent = *in_edges.begin();

        set<mempath> mempaths_new;
        auto accumulate_all = [&]() {
          //          cout << "This call requires the following fields:" << endl;
          //           cout << "Reqs for call to " << address << endl;
          (this->path_construction_errors[from])[0] = 0;
          for(auto &req : desc.field_reqs) {
            //                        cout << req << endl;
            optional<set<mempath>> mempaths_new_next = nullopt;
            auto prop_res = req.propagate(mempaths_new_next,
              get_sub(from_parent, from_ctx)->get_mstate(), state_new->get_mstate());
            if(mempaths_new_next)
              mempaths_new.insert(mempaths_new_next->begin(), mempaths_new_next->end());

            for(auto ptr : prop_res.constant_ptrs) {
              //               cout << "\tNew immediate ptr: " << ptr << endl;
              (this->pointer_props[(size_t)address])[req].insert(ptr);
              ((this->hb_counts[to]))[req].insert(ptr);
            }
            (this->path_construction_errors[from])[0] += prop_res.path_construction_errors;
          }
        };

        auto tabulate_all = [&]() {
          /*
           * Todo: Tabulate aliasing conflicts; a summary is specialized if and only if
           * a pointer from the conflict set aliases with another pointer in the conflict
           * set in the caller...
           */

          std::vector<std::set<mempath_assignment>> assignments_sets;
          for(auto & [ context, from_parent_state ] : get_ctxful(from_parent)) {
            auto s = dynamic_pointer_cast<global_state>(from_parent_state);
            size_t path_construction_errors = 0;
            auto keys_new =
              tabulation_keys(mempaths_new, path_construction_errors, desc, s->get_mstate());
            (this->path_construction_errors[from])[context] = path_construction_errors;
            assignments_sets.insert(assignments_sets.end(), keys_new.begin(), keys_new.end());
          }
          // cout << "****** assignments start" << endl;
          for(auto assignments_set : assignments_sets) {
            // cout << "<<< assignment" << endl;
            for(const auto& assignment : assignments_set)
              cout << assignment << endl;
            // cout << ">>> assignment" << endl;

            if(assignments_set.size() == 0) {
              /*
               * No tabulation without field requests
               */
              continue;
            }
            auto desc_it = desc.contexts.find(assignments_set);
            if(desc_it == desc.contexts.end()) {
              size_t context = desc.contexts.size() + 1;
              desc.contexts[assignments_set] = context;

              auto state_ctx = dynamic_pointer_cast<global_state>(
                start_state(vs_finite::single((int64_t)address)));
              (this->path_construction_errors[from])[context] = 0;
              for(auto assignment : assignments_set) {
                assignment.propagate(state_ctx->get_mstate());
                // (this->path_construction_errors[from])[context] +=
                //   prop_res.path_construction_errors;
              }

              state_map_new[context] = state_ctx;
            }
          }
          // cout << "****** assignments end" << endl;
        };

        if(tabulation)
          tabulate_all();
        else
          accumulate_all();

        if(mempaths_new.size() > 0) {
          //               cout << "Propagating..." << endl;
          //               for(auto p : mempaths_new.value())
          //                 cout << p << endl;

          set<void *> f_addrs = unpack_f_addrs(get_sub(from_parent, from_ctx)->get_f_addr());
          assert(f_addrs.size() > 0);
          for(auto f_addr : f_addrs)
            propagate_reqs(mempaths_new, f_addr);
        }
      }

      if(tabulation)
        state_map_new[0] = state_new;
      else
        state_map_new[from_ctx] = state_new;
    } else {
      auto state_new = get_sub(from, from_ctx);
      state_map_new[from_ctx] = state_new;
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
    result = state_map_new;
  });
  e->accept(ev);
  return result;
}

depdant_desc analysis::summary_dstack::dependants(size_t node_id) {
  depdant_desc result;
  bool end_node = get_cfg()->out_edge_payloads(node_id)->size() == 0;
  if(end_node)
    for(size_t depdant : _dependants[node_id])
      for(auto &ctx_mapping : state[depdant])
        result.context_deps[ctx_mapping.first].insert(depdant);
  else
    result.context_free_deps = _dependants[node_id];
  return result;
}

dependency analysis::summary_dstack::gen_dependency(size_t from, size_t to) {
  // Todo: Should this be here?
  node_visitor nv;
  nv._([&](address_node *_) {
    ref(from, nullopt);
    ref(to, nullopt);
  });
  cfg->get_node_payload(to)->accept(nv);

  //  cout << "Generating dep. " << from << " to " << to << endl;
  return dependency{from, to};
}

void analysis::summary_dstack::init_state(summy::vs_shared_t f_addr) {
  //  cout << "init_state()" << endl;

  size_t old_size = state.size();
  state.resize(cfg->node_count());
  for(size_t i = old_size; i < cfg->node_count(); i++) {
    //     if(fixpoint_pending.find(i) != fixpoint_pending.end()) {
    //       (state[i])[0] = dynamic_pointer_cast<global_state>(start_value(f_addr));
    //       ref(i, nullopt);
    //     } else
    (state[i])[0] = dynamic_pointer_cast<global_state>(bottom());
  }
}

std::vector<std::set<mempath_assignment>> analysis::summary_dstack::tabulation_keys(
  std::set<mempath> &remaining, size_t &path_construction_errors, function_desc const &desc,
  summary_memory_state *state) {
  std::vector<std::set<mempath_assignment>> result;

  std::vector<std::vector<mempath_assignment>> assignments;

  if(desc.field_reqs.size() == 0) {
    result.push_back({});
    return result;
  }
  for(auto &f : desc.field_reqs) {
    auto ext_res = f.extract_table_keys(state);
    path_construction_errors += ext_res.path_construction_errors;
    if(ext_res.remaining) {
      /*
       * We log requests that we could not satisfy.
       */
      remaining.insert(ext_res.remaining->begin(), ext_res.remaining->end());
    }
    if(ext_res.assignments.size() == 0)
      // Unresolved pointer => no instantiation
      return result;
    assignments.push_back(ext_res.assignments);
  }

  //   cout << "Propagating to address " << address << std::endl;

  std::vector<size_t> indices(assignments.size(), 0);
  auto entry = [&]() {
    std::set<mempath_assignment> assignments_set;
    for(int i = 0; i < assignments.size(); i++) {
      assert(assignments[i].size() > indices[i]);
      assignments_set.insert((assignments[i])[indices[i]]);
    }

    result.push_back(assignments_set);

    //               cout << "°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°" << std::endl;
    //               cout << *state_ctx << std::endl;
    //               cout << "°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°" << std::endl;
  };

  while(true) {
    entry();
    size_t digit;
    for(digit = indices.size(); digit > 0; digit--) {
      indices[digit - 1] = (indices[digit - 1] + 1) % assignments[digit - 1].size();
      if(indices[digit - 1] > 0) break;
    }
    if(!digit && indices[0] == 0) break;
  }

  return result;
}

void analysis::summary_dstack::init_state() {
  init_state(value_set::bottom);
}

analysis::summary_dstack::summary_dstack(cfg::cfg *cfg, std::shared_ptr<static_memory> sm,
  bool warnings, std::set<size_t> const &f_starts, bool tabulation)
    : fp_analysis(cfg, analysis_direction::FORWARD), sm(sm), warnings(warnings),
      stubs(sm, warnings), tabulation(tabulation) {
  init();

  for(auto node_id : f_starts) {
    node *n = cfg->get_node_payload(node_id);
    optional<size_t> addr;
    node_visitor nv;
    nv._([&](address_node *an) { addr = an->get_address(); });
    n->accept(nv);
    function_desc_map.insert(make_pair((void *)addr.value(), function_desc(0, n->get_id())));
  }
}

analysis::summary_dstack::summary_dstack(
  cfg::cfg *cfg, std::shared_ptr<static_memory> sm, bool warnings, bool tabulation)
    : fp_analysis(cfg, analysis_direction::FORWARD), sm(sm), warnings(warnings),
      stubs(sm, warnings), tabulation(tabulation) {
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
}

analysis::summary_dstack::summary_dstack(cfg::cfg *cfg, bool warnings)
    : summary_dstack(cfg, make_shared<static_dummy>(), warnings, false) {
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

std::shared_ptr<analysis::domain_state> analysis::summary_dstack::start_state(size_t node_id) {
  node *n = cfg->get_node_payload(node_id);
  optional<size_t> addr;
  node_visitor nv;
  nv._([&](address_node *an) { addr = an->get_address(); });
  n->accept(nv);

  assert(addr);
  assert(function_desc_map.find((void *)*addr) != function_desc_map.end());

  std::shared_ptr<analysis::domain_state> state =
    dynamic_pointer_cast<global_state>(start_state(vs_finite::single(addr.value())));

  return state;
}

std::shared_ptr<domain_state> analysis::summary_dstack::start_state(vs_shared_t f_addr) {
  return shared_ptr<domain_state>(new global_state(sms_top(), f_addr));
}

shared_ptr<domain_state> analysis::summary_dstack::get(size_t node) {
  //  if(node >= state.size())
  //    return bottom();
  //  assert(erased.find(node) == erased.end());
  return state[node].at(0);
}

std::map<size_t, shared_ptr<domain_state>> analysis::summary_dstack::get_ctxful(size_t node) {
  if(node >= state.size()) return {{0, dynamic_pointer_cast<global_state>(bottom())}};
  std::map<size_t, shared_ptr<domain_state>> r;
  for(auto const &entry : state.at(node))
    r[entry.first] = entry.second;
  return r;
}

shared_ptr<global_state> analysis::summary_dstack::get_sub(size_t node, size_t ctx) {
  //  if(node >= state.size())
  //    return bottom();
  // override  assert(erased.find(node) == erased.end());
  auto state = this->state[node];
  auto it = state.find(ctx);
  if(it == state.end())
    return dynamic_pointer_cast<global_state>(bottom());
  else
    return it->second;
}

void analysis::summary_dstack::update(analysis_node node, shared_ptr<domain_state> state) {
  (this->state[node.id])[node.context] = dynamic_pointer_cast<global_state>(state);
  //  erased.erase(node);
  //  this->state[node]->get_mstate()->check_consistency();
}

summary_dstack_result analysis::summary_dstack::result() {
  return summary_dstack_result(state);
}

node_compare_t analysis::summary_dstack::get_fixpoint_node_comparer() {
  return [=](analysis_node const &a, analysis_node const &b) -> optional<bool> {
    if(a.id >= state.size() || b.id >= state.size()) return a < b;
    shared_ptr<global_state> state_a = this->state[a.id].at(0);
    shared_ptr<global_state> state_b = this->state[b.id].at(0);
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

depdant_desc analysis::summary_dstack::dirty_nodes() {
  return std::move(this->_dirty_nodes);
}

void analysis::summary_dstack::check_consistency() {
  /*
   * Todo: Mittels einer extra Knotenliste soll sichergestellt werden,
   * dass Felder nicht verschwinden zwischen den Iterationen
   */
}

void analysis::summary_dstack::ref(size_t node, std::optional<size_t> count) {
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
        (state[node])[0] = dynamic_pointer_cast<global_state>(bottom());
        //        erased.insert(node);
        ref_map.erase(node);
      }
    }
  }
}

optional<size_t> analysis::summary_dstack::get_lowest_function_address(size_t node_id) {
  if(this->state.size() <= node_id) return nullopt;
  set<void *> f_addrs = unpack_f_addrs(state[node_id].at(0)->get_f_addr());
  if(f_addrs.size() == 0) return nullopt;
  return (size_t)*f_addrs.begin();
}

void analysis::summary_dstack::print_callstack(size_t node_id) {
  //  cout << *state[node_id] << endl;
  if(this->state.size() <= node_id) return;
  set<size_t> callers_current_heads = get_function_heads(state[node_id].at(0));
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
      set<size_t> callers_next = get_callers(state[caller].at(0));
      for(auto caller_next : callers_next) {
        set<size_t> callers_next_heads_current = get_function_heads(state[caller_next].at(0));
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
    out << "Node " << i << ": " << endl << *state[i].at(0) << endl;
}
