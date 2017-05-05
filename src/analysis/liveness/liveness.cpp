
/*
 * liveness.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#include <assert.h>
#include <cppgdsl/rreil/rreil.h>
#include <cppgdsl/rreil/statement/statement_visitor.h>
#include <functional>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <summy/analysis/domain_state.h>
#include <summy/analysis/liveness/liveness.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <summy/cfg/edge/phi_edge.h>
#include <summy/rreil/copy_visitor.h>
#include <summy/rreil/visitor.h>
#include <summy/tools/rreil_util.h>
#include <tuple>
#include <vector>

using namespace std;
using namespace cfg;
using namespace analysis::liveness;
using namespace gdsl::rreil;
namespace sr = summy::rreil;

static long long unsigned int range(unsigned long long offset, unsigned long long size) {
  if(size + offset > 64 || offset > 64)
    throw string("Such a big size/offset is not yet implemented");
  //    return (long long unsigned)(-1);
  long long unsigned r = (size + offset == 64 ? ((unsigned long long)(-1))
                                              : (((unsigned long long)1 << (size + offset)) - 1)) &
                         ~(((unsigned long long)1 << offset) - 1);
  //  cout << "range for " << offset << "/" << size << ": " <<  r << endl;
  return r;
}

bool analysis::liveness::liveness_result::contains(
  size_t node_id, singleton_key_t sk, unsigned long long offset, unsigned long long size) {
  return contains(node_id, singleton_t(sk, range(offset, size)));
}

bool analysis::liveness::liveness_result::contains(size_t node_id, singleton_t s) {
  return result[node_id]->contains_bit(s);
}

void analysis::liveness::liveness::add_constraint(size_t from, size_t to, const ::cfg::edge *e) {
  function<shared_ptr<lv_state>()> transfer_f = [=]() { return state[to]; };
  auto acc_newly_live = [&](int_t size, function<void(visitor &)> accept_live) {
    sr::copy_visitor cv;
    vector<singleton_t> newly_live;
    sr::visitor id_acc;
    id_acc._((function<void(variable const *)>)([&](variable const *var) {
      var->get_id().accept(cv);
      newly_live.push_back(singleton_t(
        shared_ptr<gdsl::rreil::id>(cv.retrieve_id().release()), range(var->get_offset(), size)));
    }));
    accept_live(id_acc);
    pn_newly_live[from].insert(pn_newly_live[from].end(), newly_live.begin(), newly_live.end());
    return newly_live;
  };
  auto assignment = [&](int_t size, variable const *assignee, vector<singleton_t> newly_live) {
    sr::copy_visitor cv;
    assignee->get_id().accept(cv);
    singleton_t lhs(
      shared_ptr<gdsl::rreil::id>(cv.retrieve_id().release()), range(assignee->get_offset(), size));

    transfer_f = [=]() {
      auto current_state = transfer_f();
      bool edge_live = current_state->contains_bit(lhs);
      this->edge_liveness[edge_id(from, to)] = edge_live;
      if(edge_live) {
        shared_ptr<lv_state> dead_removed(current_state->remove({lhs}));
        return shared_ptr<lv_state>(dead_removed->add(newly_live));
      } else
        return current_state;
    };
  };
  auto memory_phi_ass = [&](const phi_memory &m) {
    if(m.from != m.to)
      transfer_f = [=]() {
        auto current_state = transfer_f();
        this->edge_liveness[edge_id(from, to)] = true;
        return current_state;
      };
  };
  auto access = [&](vector<singleton_t> newly_live) {
    transfer_f = [=]() {
      this->edge_liveness[edge_id(from, to)] = true;
      return shared_ptr<lv_state>(transfer_f()->add(newly_live));
    };
  };
  edge_visitor ev;
  ev._([&](const stmt_edge *edge) {
    statement *stmt = edge->get_stmt();
    statement_visitor v;
    v._([&](assign const *i) {
      auto accept_live = [&](visitor &v) { i->get_rhs().accept(v); };
      auto newly_live = acc_newly_live(rreil_prop::size_of_rhs(i), accept_live);
      assignment(i->get_size(), &i->get_lhs(), newly_live);
    });
    v._([&](load const *l) {
      auto accept_live = [&](visitor &v) { l->get_address().get_lin().accept(v); };
      auto newly_live = acc_newly_live(l->get_address().get_size(), accept_live);
      assignment(l->get_size(), &l->get_lhs(), newly_live);
    });
    v._([&](store const *s) {
      function<void(visitor &)> accept_live = [&](
        visitor &v) { s->get_address().get_lin().accept(v); };
      auto newly_live = acc_newly_live(s->get_address().get_size(), accept_live);
      access(newly_live);
      accept_live = [&](visitor &v) { s->get_rhs().accept(v); };
      newly_live = acc_newly_live(s->get_size(), accept_live);
      access(newly_live);
    });
    v._([&](cbranch const *c) {
      auto newly_live = acc_newly_live(1, [&](visitor &v) { c->get_cond().accept(v); });
      access(newly_live);
      newly_live = acc_newly_live(
        c->get_target_true().get_size(), [&](visitor &v) { c->get_target_true().accept(v); });
      access(newly_live);
      newly_live = acc_newly_live(
        c->get_target_false().get_size(), [&](visitor &v) { c->get_target_false().accept(v); });
      access(newly_live);
    });
    v._([&](branch const *b) {
      auto newly_live =
        acc_newly_live(b->get_target().get_size(), [&](visitor &v) { b->get_target().accept(v); });
      access(newly_live);
    });
    v._([&](floating const *_) { throw string("Not implemented"); });
    v._([&](prim const *_) {
      //      throw string("Not implemented");
    });
    v._([&](_throw const *_) {
      //      throw string("Not implemented");
    });
    v._default([&](statement const *_) { throw string("Should not happen :/"); });
    stmt->accept(v);
  });
  ev._([&](const cond_edge *edge) {
    sexpr *cond = edge->get_cond();
    auto newly_live = acc_newly_live(1, [&](visitor &v) { cond->accept(v); });
    access(newly_live);
  });
  ev._([&](const phi_edge *edge) {
    for(auto const &ass : edge->get_assignments()) {
      auto accept_live = [&](visitor &v) { ass.get_rhs().accept(v); };
      auto newly_live = acc_newly_live(ass.get_size(), accept_live);
      assignment(ass.get_size(), &ass.get_lhs(), newly_live);
    }
    memory_phi_ass(edge->get_memory());
  });
  e->accept(ev);
  (constraints[from])[to] = [=](size_t) { return default_context(transfer_f()); };
}

void analysis::liveness::liveness::remove_constraint(size_t from, size_t to) {
  (constraints[from]).erase(to);
}

analysis::dependency analysis::liveness::liveness::gen_dependency(size_t from, size_t to) {
  return dependency{to, from};
}

void analysis::liveness::liveness::init_state() {
  size_t old_size = state.size();
  state.resize(cfg->node_count());
  for(size_t i = old_size; i < cfg->node_count(); i++)
    state[i] = dynamic_pointer_cast<lv_state>(bottom());
}

analysis::liveness::liveness::liveness(class cfg *cfg) : fp_analysis(cfg) {
  init();
}

analysis::liveness::liveness::~liveness() {}

shared_ptr<analysis::domain_state> analysis::liveness::liveness::bottom() {
  return make_shared<lv_state>(elements_t{});
}

shared_ptr<analysis::domain_state> analysis::liveness::liveness::get(size_t node) {
  return state[node];
}

void analysis::liveness::liveness::update(size_t node, shared_ptr<domain_state> state) {
  this->state[node] = dynamic_pointer_cast<lv_state>(state);
}

liveness_result analysis::liveness::liveness::result() {
  return liveness_result(state, pn_newly_live, edge_liveness);
}

void analysis::liveness::liveness::put(std::ostream &out) {
  for(size_t i = 0; i < state.size(); i++) {
    out << i << ": " << *state[i] << "; newly live: {";
    for(size_t j = 0; j < pn_newly_live[i].size(); j++) {
      if(j) out << ", ";
      singleton_key_t k;
      singleton_value_t v;
      tie(k, v) = (pn_newly_live[i])[j];
      out << "(" << *k << ", " << hex << v << dec << ")";
    }
    out << "}" << endl;
  }
  for(auto &eid_it : this->edge_liveness)
    cout << eid_it.first.from << " -> " << eid_it.first.to << ": "
         << (eid_it.second ? "live" : "dead") << endl;
}
