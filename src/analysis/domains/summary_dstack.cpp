/*
 * summary_summary_dstack.cpp
 *
 *  Created on: Mar 17, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/summary_dstack.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <cppgdsl/rreil/statement/statement.h>
#include <summy/analysis/domains/numeric/als_state.h>
#include <summy/analysis/domains/numeric/equality_state.h>
#include <summy/analysis/domains/numeric/vsd_state.h>
#include <summy/analysis/domains/api/api.h>
#include <summy/analysis/static_memory.h>
#include <summy/value_set/vs_finite.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/node/address_node.h>
#include <summy/cfg/node/node.h>
#include <summy/cfg/node/node_visitor.h>
#include <summy/cfg/observer.h>
#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/sm_id.h>
#include <summy/value_set/value_set_visitor.h>
#include <functional>
#include <assert.h>
#include <experimental/optional>

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

bool analysis::summary_dstack::unpack_f_addr(void *&r, summy::vs_shared_t f_addr) {
  bool single = false;
  value_set_visitor vsv;
  vsv._([&](vs_finite *vsf) {
    vs_finite::elements_t const &elems = vsf->get_elements();
    if(elems.size() == 1) {
      single = true;
      r = (void*)*elems.begin();
    }
  });
  f_addr->accept(vsv);
  return single;
}

void analysis::summary_dstack::add_constraint(size_t from, size_t to, const ::cfg::edge *e) {
//  cout << "Adding constraint from " << from << " to " << to << endl;

  function<shared_ptr<global_state>()> transfer_f = [=]() {
    return state[from];
  };
  auto for_mutable = [&](function<void(summary_memory_state*)> cb) {
    transfer_f = [=]() {
      shared_ptr<global_state> &state_c = this->state[from];
      summary_memory_state *mstate_new = state_c->get_mstate()->copy();
      cb(mstate_new);
      shared_ptr<global_state> global_new = shared_ptr<global_state>(
          new global_state(mstate_new, state_c->get_f_addr(), state_c->get_callers()));
      return global_new;
    };
  };
  auto for_update = [&](auto *update) {
    for_mutable([=](summary_memory_state *state_new) {
      state_new->update(update);
    });
  };
  edge_visitor ev;
  ev._([&](const stmt_edge *edge) {
    statement *stmt = edge->get_stmt();
    statement_visitor v;
    v._([&](assign *a) {
      for_update(a);
    });
    v._([&](load *l) {
      for_update(l);
    });
    v._([&](store *s) {
      for_update(s);
    });
    v._([&](branch *b) {
      switch(b->get_hint()) {
        case gdsl::rreil::BRANCH_HINT_CALL: {
          transfer_f = [=]() {
            shared_ptr<global_state> state_c = this->state[from];
            summary_memory_state *mstate = state_c->get_mstate();

            void *f_addr;
            bool unpackable_a = unpack_f_addr(f_addr, state_c->get_f_addr());
            assert(unpackable_a);
            size_t current_min_calls_sz = function_desc_map.at(f_addr).min_calls_sz;

            shared_ptr<summary_memory_state> summary = shared_ptr<summary_memory_state>(sms_bottom());

            ptr_set_t callee_aliases = mstate->queryAls(b->get_target());
            for(auto ptr : callee_aliases) {
              summy::rreil::id_visitor idv;
              bool is_text = false;
              void *text_address;
              idv._([&] (sm_id *sid) {
                if(sid->get_symbol() == ".text") {
                  is_text = true;
                  text_address = sid->get_address();
                }
              });
              ptr.id->accept(idv);
              if(!is_text)
                continue;
              value_set_visitor vsv;
              vsv._([&](vs_finite *vsf) {
                for(int64_t offset : vsf->get_elements()) {
                  offset /= 8;
                  void *address = (char*)text_address + offset;
                  auto fd_it = function_desc_map.find(address);
                  if(fd_it != function_desc_map.end()) {
                    auto &f_desc = fd_it->second;
                    summary = shared_ptr<summary_memory_state>(summary->join(f_desc.summary.get(), to));
                    f_desc.min_calls_sz = std::max(f_desc.min_calls_sz, current_min_calls_sz);

                    size_t head_id = fd_it->second.head_id;
                    cfg::in_edges_t const &head_in = cfg->in_edges(head_id);
                    if(head_in.find(to) == head_in.end()) {
//                      cout << "New edge from " << to << " to " << head_id << endl;
                      cfg->update_edge(to, head_id, new call_edge(true));
                    }
                  } else {
                    size_t an_id = cfg->create_node([&](size_t id) {
                      return new address_node(id, (size_t)address, cfg::DECODABLE);
                    });
                    function_desc_map.insert(make_pair(address, function_desc(summary_t(sms_bottom()), current_min_calls_sz + 1, an_id)));
//                    cout << "New edge from " << to << " to " << an_id << endl;
                    cfg->update_edge(to, an_id, new call_edge(true));
                  }
                }
              });
              ptr.offset->accept(vsv);
//              cout << ptr << endl;
            }
            cfg->commit_updates();

//            cout << "Need to apply the following summary: " << endl;
//            cout << *summary << endl;

            summary_memory_state *summarized = mstate->apply_summary(summary.get());
            return shared_ptr<global_state>(new global_state(summarized, state_c->get_f_addr(), state_c->get_callers()));
          };
          break;
        }
        case gdsl::rreil::BRANCH_HINT_RET: {
          transfer_f = [=]() {
            shared_ptr<global_state> state_c = this->state[from];
            void *f_addr;
            bool unpackable_a = unpack_f_addr(f_addr, state_c->get_f_addr());
            assert(unpackable_a);
            auto fd_it = function_desc_map.find(f_addr);
//            if(summary_it == summary_map.end())
//              summary_map[state_c->get_f_addr()] = shared_ptr<summary_memory_state>(state_c->get_mstate()->copy());
//            else
            fd_it->second.summary = shared_ptr<summary_memory_state>(fd_it->second.summary->join(state_c->get_mstate(), to));
            for(auto caller : state_c->get_callers())
              assert_dependency(gen_dependency(to, caller));
            return state_c;
          };
          break;
        }
        case gdsl::rreil::BRANCH_HINT_JUMP: {
          break;
        }
      }
    });
    stmt->accept(v);
  });
  ev._([&](const cond_edge *edge) {
    for_mutable([=](summary_memory_state *state_new) {
      if(edge->is_positive())
        state_new->assume(edge->get_cond());
      else
        state_new->assume_not(edge->get_cond());
    });
  });
  ev._([&](const call_edge *edge) {
    transfer_f = [=]() {
      shared_ptr<global_state> state_new;
      if(edge->is_target_edge()) {
        node *dest = cfg->get_node_payload(to);
        bool is_addr_node = false;
        void *address;
        node_visitor nv;
        nv._([&](address_node *an) {
          is_addr_node = true;
          address = (void*)an->get_address();
        });
        dest->accept(nv);
        assert(is_addr_node);
        state_new = dynamic_pointer_cast<global_state>(start_value(vs_finite::single((int64_t)address),  callers_t {from}));
      } else {
        state_new = state[from];
        if(!state_new->get_mstate()->is_bottom()) {
          bool needs_decoding = false;
          size_t addr;
          node_visitor nv;
          nv._([&](address_node *av) {
            if(av->get_decs() != cfg::DECODED) {
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
      return state_new;
    };
  });
  e->accept(ev);
  (constraints[to])[from] = transfer_f;
}

void analysis::summary_dstack::remove_constraint(size_t from, size_t to) {
  constraints[to].erase(from);
}

dependency analysis::summary_dstack::gen_dependency(size_t from, size_t to) {
//  cout << "Generating dep. " << from << " to " << to << endl;
  return dependency { from, to };
}

void analysis::summary_dstack::init_state(summy::vs_shared_t f_addr) {
//  cout << "init_state()" << endl;

  size_t old_size = state.size();
  state.resize(cfg->node_count());
  for(size_t i = old_size; i < cfg->node_count(); i++) {
    if(fixpoint_pending.find(i) != fixpoint_pending.end()) state[i] = dynamic_pointer_cast<global_state>(start_value(f_addr));
    else state[i] = dynamic_pointer_cast<global_state>(bottom());
  }
}

void analysis::summary_dstack::init_state() {
  init_state(value_set::bottom);
}

analysis::summary_dstack::summary_dstack(cfg::cfg *cfg, std::shared_ptr<static_memory> sm) :
  fp_analysis(cfg), sm(sm) {
  /*
   * Todo: Use correct start address here
   */
  init();

  /*
   * Simulate initial call to node zero
   */
  node *n = cfg->get_node_payload(0);
  optional<size_t> addr;
  node_visitor nv;
  nv._([&](address_node *an) {
    addr = an->get_address();
  });
  n->accept(nv);
  function_desc_map.insert(make_pair((void*)addr.value(), function_desc(summary_t(sms_bottom()), 0, n->get_id())));
  state[n->get_id()]->set_f_addr(vs_finite::single(addr.value()));
}

analysis::summary_dstack::summary_dstack(cfg::cfg *cfg) :
    summary_dstack(cfg, make_shared<static_dummy>()) {
  init();
}

analysis::summary_dstack::~summary_dstack() {
}

summary_memory_state *analysis::summary_dstack::sms_bottom() {
  return summary_memory_state::bottom(sm, new equality_state(new als_state(vsd_state::bottom(sm))));;
}

summary_memory_state *analysis::summary_dstack::sms_top() {
  summary_memory_state *sms_start = summary_memory_state::start_value(sm, new equality_state(new als_state(vsd_state::top(sm))));
  return sms_start;
}

shared_ptr<domain_state> analysis::summary_dstack::bottom() {
  return shared_ptr<domain_state>(new global_state(sms_bottom(), value_set::bottom, callers_t {}));
}

std::shared_ptr<domain_state> analysis::summary_dstack::start_value(vs_shared_t f_addr, callers_t callers) {
  return shared_ptr<domain_state>(new global_state(sms_top(), f_addr, callers));
}

std::shared_ptr<domain_state> analysis::summary_dstack::start_value(vs_shared_t f_addr) {
//  cout << "start_value()" << endl;
  return start_value(f_addr, callers_t {});
}

shared_ptr<domain_state> analysis::summary_dstack::get(size_t node) {
//  if(node >= state.size())
//    return bottom();
  return state[node];
}

void analysis::summary_dstack::update(size_t node, shared_ptr<domain_state> state) {
  this->state[node] = dynamic_pointer_cast<global_state>(state);
}

summary_dstack_result analysis::summary_dstack::result() {
  return summary_dstack_result(state);
}

node_compare_t analysis::summary_dstack::get_fixpoint_node_comparer() {
  return [=](size_t a, size_t b) {
    shared_ptr<global_state> state_a = this->state[a];
    shared_ptr<global_state> state_b = this->state[b];
//    cout << state_a->get_f_addr() << " " << state_b->get_f_addr() << endl;

    void *f_addr_a;
    bool unpackable_a = unpack_f_addr(f_addr_a, state_a->get_f_addr());

    void *f_addr_b;
    bool unpackable_b = unpack_f_addr(f_addr_b, state_b->get_f_addr());

//    cout << a << " < " << b << " ~~~ " << *state_a->get_f_addr() << " //// " << *state_b->get_f_addr() << endl;

    if(unpackable_a && !unpackable_b)
      return true;
    else if(!unpackable_a && unpackable_b)
      return false;
    else if(unpackable_a && unpackable_b) {
      size_t min_calls_sz_a = function_desc_map.at(f_addr_a).min_calls_sz;
      size_t min_calls_sz_b = function_desc_map.at(f_addr_b).min_calls_sz;
//      cout << a << " < " << b << ";;; " << min_calls_sz_a << " //// " << min_calls_sz_b << endl;
      if(min_calls_sz_a > min_calls_sz_b)
        return true;
      else if(min_calls_sz_a < min_calls_sz_b)
        return false;
      else
        return a < b;
    }
    else return a < b;
  };
}

void analysis::summary_dstack::put(std::ostream &out) {
  for(size_t i = 0; i < state.size(); i++)
    out << "Node " << i << ": " << endl << *state[i] << endl;
}
