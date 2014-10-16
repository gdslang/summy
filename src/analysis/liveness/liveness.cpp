
/*
 * liveness.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/liveness/liveness.h>
#include <summy/tools/rreil_util.h>
#include <summy/analysis/lattice_elem.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/edge.h>
#include <cppgdsl/rreil/rreil.h>
#include <cppgdsl/rreil/statement/statement_visitor.h>
#include <cppgdsl/rreil/copy_visitor.h>
#include <functional>
#include <memory>
#include <vector>
#include <set>
#include <tuple>
#include <iostream>
#include <assert.h>

using namespace std;
using namespace cfg;
using namespace analysis::liveness;
using namespace gdsl::rreil;

static long long unsigned int range(unsigned long long offset, unsigned long long size) {
  if(size + offset > 64 || offset > 64)
    throw string("Such a big size/offset is not yet implemented");
  long long unsigned r = (
      size + offset == 64 ? ((unsigned long long)(-1)) : (((unsigned long long)1 << (size + offset)) - 1))
      & ~(((unsigned long long)1 << offset) - 1);
  cout << "range for " << offset << "/" << size << ": " <<  r << endl;
  return r;
}

bool analysis::liveness::liveness_result::contains(size_t node_id, singleton_key_t sk, unsigned long long offset,
    unsigned long long size) {
  return contains(node_id, singleton_t(sk, range(offset, size)));
}

bool analysis::liveness::liveness_result::contains(size_t node_id, singleton_t s) {
  return result[node_id]->contains_bit(s);
}

void analysis::liveness::liveness::init_constraints() {
  for(auto node : *cfg) {
    size_t node_id = node->get_id();
    vector<constraint_t> node_constraints;
    auto &edges = *cfg->out_edges(node_id);
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      size_t dest_node = edge_it->first;
      constraint_t transfer_f = [=]() {
        return state[dest_node];
      };
      auto acc_newly_live = [&](int_t size, function<void(visitor&)> accept_live) {
        copy_visitor cv;
        vector<singleton_t> newly_live;
        visitor id_acc;
        id_acc._((function<void(variable*)>)([&](variable *var) {
          var->get_id()->accept(cv);
          newly_live.push_back(singleton_t(shared_ptr<gdsl::rreil::id>(cv.get_id()), range(var->get_offset(), size)));
        }));
        accept_live(id_acc);
        pn_newly_live[node_id].insert(pn_newly_live[node_id].end(), newly_live.begin(), newly_live.end());
        return newly_live;
      };
      auto assignment = [&](int_t size, variable *assignee, vector<singleton_t> newly_live) {
        copy_visitor cv;
        assignee->get_id()->accept(cv);
        singleton_t lhs(shared_ptr<gdsl::rreil::id>(cv.get_id()), range(assignee->get_offset(), size));

        size_t foo = assignee->get_offset();

        transfer_f = [=]() {
//          cout << *lhs << endl;
          cout << "dest: " << dest_node << " offset: " << foo << " size: " << size << " id: " << *(std::get<0>(lhs)) << endl;
          if(state[dest_node]->contains_bit(lhs)) {
          cout << "dest: " << dest_node << " offset: " << foo << " size: " << size << endl;
            shared_ptr<lv_elem> dead_removed(state[dest_node]->remove({ lhs }));
//            cout << "after dead_removed: " << *dead_removed << endl;
            return shared_ptr<lv_elem>(dead_removed->add(newly_live));
          } else
            return state[dest_node];
        };
      };
      auto access = [&](vector<singleton_t> newly_live) {
        transfer_f = [=]() {
          return shared_ptr<lv_elem>(state[dest_node]->add(newly_live));
        };
      };
      edge_visitor ev;
      ev._([&](stmt_edge *edge) {
        statement *stmt = edge->get_stmt();
        statement_visitor v;
        v._([&](assign *i) {
          auto accept_live = [&](visitor &v) {
            i->get_rhs()->accept(v);
          };
          int_t size = rreil_prop::size_of_assign(i);
          auto newly_live = acc_newly_live(size, accept_live);
          assignment(size, i->get_lhs(), newly_live);
        });
        v._([&](load *l) {
          auto accept_live = [&](visitor &v) {
            l->get_address()->get_lin()->accept(v);
          };
          auto newly_live = acc_newly_live(l->get_address()->get_size(), accept_live);
          assignment(l->get_size(), l->get_lhs(), newly_live);
        });
        v._([&](store *s) {
          function<void(visitor&)> accept_live = [&](visitor &v) {
            s->get_address()->get_lin()->accept(v);
          };
          acc_newly_live(s->get_address()->get_size(), accept_live);
          accept_live = [&](visitor &v) {
            s->get_rhs()->accept(v);
          };
          acc_newly_live(s->get_size(), accept_live);
        });
        v._([&](cbranch *c) {
          auto newly_live = acc_newly_live(1, [&](visitor &v) {
            c->get_cond()->accept(v);
          });
          access(newly_live);
          newly_live = acc_newly_live(c->get_target_true()->get_size(), [&](visitor &v) {
            c->get_target_true()->accept(v);
          });
          access(newly_live);
          newly_live = acc_newly_live(c->get_target_false()->get_size(), [&](visitor &v) {
            c->get_target_false()->accept(v);
          });
          access(newly_live);
        });
        v._([&](branch *b) {
          auto newly_live = acc_newly_live(b->get_target()->get_size(), [&](visitor &v) {
            b->get_target()->accept(v);
          });
          access(newly_live);
        });
        v._([&](floating *_) {
          throw string("Not implemented");
        });
        v._([&](prim *_) {
          throw string("Not implemented");
        });
        v._([&](_throw *_) {
          throw string("Not implemented");
        });
        v._default([&](statement *_) {
          throw string("Should not happen :/");
        });
        stmt->accept(v);
      });
      ev._([&](cond_edge *edge) {
        sexpr *cond = edge->get_cond();
        auto newly_live = acc_newly_live(1, [&](visitor &v) {
          cond->accept(v);
        });
        access(newly_live);
      });
      edge_it->second->accept(ev);
      constraints[node_id].push_back(transfer_f);
    }
  }
}

void analysis::liveness::liveness::init_dependants() {
  for(auto node : *cfg) {
    size_t node_id = node->get_id();
    auto &edges = *cfg->out_edges(node_id);
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++)
      _dependants[edge_it->first].insert(node_id);
  }
}

analysis::liveness::liveness::liveness(class cfg *cfg) : analysis(cfg), pn_newly_live(cfg->node_count()) {
  init();

  state = state_t(cfg->node_count());
  for(size_t i = 0; i < state.size(); i++)
    state[i] = dynamic_pointer_cast<lv_elem>(bottom());
}

analysis::liveness::liveness::~liveness() {
}

shared_ptr<analysis::lattice_elem> analysis::liveness::liveness::bottom() {
  return make_shared<lv_elem>(elements_t {});
}

shared_ptr<analysis::lattice_elem> analysis::liveness::liveness::get(size_t node) {
  return state[node];
}

void analysis::liveness::liveness::update(size_t node, shared_ptr<lattice_elem> state) {
  this->state[node] = dynamic_pointer_cast<lv_elem>(state);
}

liveness_result analysis::liveness::liveness::result() {
  return liveness_result(state, pn_newly_live);
}

void analysis::liveness::liveness::put(std::ostream &out) {
  for(size_t i = 0; i < state.size(); i++) {
    out << i << ": " << *state[i] << "; newly live: {";
    for(size_t j = 0; j < pn_newly_live[i].size(); j++) {
      if(j)
        out << ", ";
      singleton_key_t k;
      singleton_value_t v;
      tie(k, v) = (pn_newly_live[i])[j];
      out << "(" << *k << ", " << v << ")";
    }
    out << "}" << endl;
  }
}


