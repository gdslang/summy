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
#include <summy/cfg/observer.h>
#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/sm_id.h>
#include <summy/value_set/value_set_visitor.h>
#include <functional>

using cfg::address_node;
using cfg::cond_edge;
using cfg::decoding_state;
using cfg::edge_visitor;
using cfg::node;
using cfg::recorder;
using cfg::stmt_edge;
using summy::value_set_visitor;
using summy::vs_finite;

using namespace analysis;
using namespace std;
using namespace gdsl::rreil;
using namespace analysis::value_sets;
using namespace api;
using namespace summy::rreil;

void analysis::summary_dstack::add_constraint(size_t from, size_t to, const ::cfg::edge *e) {
  cout << "Adding constraint from " << from << " to " << to << endl;

  function<shared_ptr<global_state>()> transfer_f = [=]() {
    return state[from];
  };
  auto for_mutable = [&](function<void(summary_memory_state*)> cb) {
    transfer_f = [=]() {
      shared_ptr<global_state> &state_c = this->state[from];
      summary_memory_state *mstate_new = state_c->get_mstate()->copy();
      cb(mstate_new);
      shared_ptr<global_state> global_new = shared_ptr<global_state>(
          new global_state(mstate_new, state_c->get_fstart_id(), state_c->get_callers()));
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
            shared_ptr<global_state> &state_c = this->state[from];
            summary_memory_state *cons = state_c->get_mstate();

            ptr_set_t callee_aliases = cons->queryAls(b->get_target());
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
                  void *address = (char*)text_address + offset;

                  size_t an_id = cfg->create_node([&](size_t id) {
                    return new address_node(id, (size_t)address, cfg::DECODABLE);
                  });
                  cfg->update_edge(from, an_id, new cond_edge(new sexpr_lin(new lin_imm(0)), true));
                }
              });
              ptr.offset->accept(vsv);
//              cout << ptr << endl;
            }
            recorder rec(cfg);
            cfg->commit_updates();
            fp_analysis::update(rec.get_updates());
            return state_c;
          };
          break;
        }
        case gdsl::rreil::BRANCH_HINT_RET: {
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
  e->accept(ev);
  (constraints[to])[from] = transfer_f;
}

void analysis::summary_dstack::remove_constraint(size_t from, size_t to) {
  constraints[to].erase(from);
}

dependency analysis::summary_dstack::gen_dependency(size_t from, size_t to) {
  return dependency { from, to };
}

void analysis::summary_dstack::init_state() {
  size_t old_size = state.size();
  state.resize(cfg->node_count());
  for(size_t i = old_size; i < cfg->node_count(); i++) {
    if(fixpoint_pending.find(i) != fixpoint_pending.end()) state[i] = dynamic_pointer_cast<global_state>(start_value());
    else state[i] = dynamic_pointer_cast<global_state>(bottom());
  }
}

analysis::summary_dstack::summary_dstack(cfg::cfg *cfg, std::shared_ptr<static_memory> sm) :
  fp_analysis(cfg), sm(sm) {
  init();
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

shared_ptr<domain_state> analysis::summary_dstack::bottom() {
  return shared_ptr<domain_state>(new global_state(sms_bottom(), 0, callers_t {}));
}

std::shared_ptr<domain_state> analysis::summary_dstack::start_value() {
  summary_memory_state *sms_start = summary_memory_state::start_value(sm, new equality_state(new als_state(vsd_state::top(sm))));
  return shared_ptr<domain_state>(new global_state(sms_start, 0, callers_t {}));
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

void analysis::summary_dstack::put(std::ostream &out) {
  for(size_t i = 0; i < state.size(); i++)
    out << "Node " << i << ": " << endl << *state[i] << endl;
}
