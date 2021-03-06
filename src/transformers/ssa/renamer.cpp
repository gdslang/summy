/*
 * Copyright 2014-2016 Julian Kranz, Technical University of Munich
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * renamer.cpp
 *
 *  Created on: Oct 20, 2014
 *      Author: Julian Kranz
 */

#include <summy/transformers/ssa/renamer.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/edge_visitor.h>
#include <summy/rreil/id/ssa_id.h>
#include <summy/rreil/copy_visitor.h>
#include <summy/rreil/visitor.h>
#include <summy/rreil/id/id_visitor.h>

#include <cppgdsl/rreil/rreil.h>
#include <cppgdsl/rreil/statement/statement_visitor.h>
#include <summy/cfg/edge/phi_edge.h>
#include <tuple>
#include <vector>

using namespace std;
using namespace cfg;
namespace sr = summy::rreil;
using namespace gdsl::rreil;

void renamer::task_from_edge(std::vector<update_task>& tasks, size_t from, size_t to, const edge *e) {
  edge_visitor ev;

  bool update = false;
  auto add_update = [&](size_t from, size_t to, edge *e) {
    if(update)
      tasks.push_back({ e, from, to });
    else
      delete e;
  };

  sr::copy_visitor cv;
  auto rebuild = [&](auto &rd_elements, auto const& ast_node) {
    cv._([&](std::unique_ptr<id> _id, int_t offset) -> std::unique_ptr<variable> {
      shared_ptr<id> id_wrapped(_id.get(), [](auto x) {});
      auto const &rd_mapping = rd_elements.find(id_wrapped);
      if(rd_mapping != rd_elements.end()) {
        sr::id_visitor iv;
        iv._([&](sr::ssa_id const *si) {
          if(si->get_version() != rd_mapping->second) {
            sr::copy_visitor cv;
            si->get_id().accept(cv);
            _id = std::unique_ptr<id>(new sr::ssa_id(cv.retrieve_id(), rd_mapping->second));
            update = true;
          }
        });
        iv._default([&](id const *_) {
            _id = make_unique<sr::ssa_id>(std::move(_id), rd_mapping->second);
            update = true;
        });
        _id->accept(iv);
      }
      return make_variable(std::move(_id), offset);
    });
    ast_node.accept(cv);
  };

  auto &rd_dst_elements = rd_result.result[to]->get_elements();
  auto &rd_src_elements = rd_result.result[from]->get_elements();
  auto assignment = [&](variable const &lhs, auto const& rhs, auto get_rhs_from_cv, auto node_ctor) {
    rebuild(rd_dst_elements, lhs);
    auto lhs_new = cv.retrieve_variable();

    rebuild(rd_src_elements, rhs);
    auto rhs_new = get_rhs_from_cv();

    return node_ctor(std::move(lhs_new), std::move(rhs_new));
  };

  ev._([&](const stmt_edge *edge) {
    statement_visitor v;
    v._([&](assign const *a) {
      auto s = assignment(a->get_lhs(), a->get_rhs(), [&]() {
        return cv.retrieve_expr();
      }, [&](auto lhs, auto rhs) {
        return assign(a->get_size(), std::move(lhs), std::move(rhs));
      });
      add_update(from, to, new stmt_edge(&s));
    });
    v._([&](load const *l) {
      auto s = assignment(l->get_lhs(), l->get_address(), [&]() {
        return cv.retrieve_address();
      }, [&](auto lhs, auto rhs) {
        return load(l->get_size(), std::move(lhs), std::move(rhs));
      });
      add_update(from, to, new stmt_edge(&s));
    });
    v._default([&](statement const *s) {
      rebuild(rd_src_elements, *s);
      auto stmt_new = cv.retrieve_statement();
      add_update(from, to, new stmt_edge(stmt_new.get()));
    });
    edge->get_stmt()->accept(v);
  });
  ev._([&](const cond_edge *ce) {
    rebuild(rd_src_elements, *ce->get_cond());
    auto sexpr_new = cv.retrieve_sexpr();
    add_update(from, to, new cond_edge(sexpr_new.get(), ce->is_positive()));
  });
  ev._([&](const phi_edge *edge) {
    assignments_t assignments_new;
    for(auto &ass : edge->get_assignments()) {
      auto ass_new = assignment(ass.get_lhs(), ass.get_rhs(), [&]() {
        return cv.retrieve_variable();
      }, [&](auto lhs, auto rhs) {
        auto phi_ass = phi_assign(lhs.get(), rhs.get(), ass.get_size());
        return phi_ass;
      });
      assignments_new.push_back(ass_new);
    }
    size_t memory_from = rd_result.result[from]->get_memory_rev();
    size_t memory_to = rd_result.result[to]->get_memory_rev();
    if(memory_from != memory_to)
      update = true;
    add_update(from, to, new phi_edge(assignments_new, phi_memory(memory_from, memory_to)));
  });
  e->accept(ev);
}

void renamer::transform(std::vector<update_task> &tasks) {
//  cout << "tasks: " << tasks.size() << endl;
  for(auto &update : tasks) {
    cfg->update_destroy_edge(update.from, update.to, update.e);
  }
}

void renamer::transform() {
  /*
   * The following throw statement
   * allows GDB to load summy (o.O)
   */
//  throw("flah");

  vector<update_task> tasks;
  for(auto node : *cfg) {
    size_t from = node->get_id();
    auto &edges = *cfg->out_edge_payloads(node->get_id());
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      size_t to = edge_it->first;
      task_from_edge(tasks, from, to, edge_it->second);
    }
  }
  transform(tasks);
}

void renamer::update(std::set<std::tuple<size_t, size_t>> &updates) {
  vector<update_task> tasks;
  for(auto &update : updates) {
    size_t from;
    size_t to;
    tie(from, to) = update;
    if(cfg->contains_edge(from, to)) task_from_edge(tasks, from, to, cfg->out_edge_payloads(from)->at(to));
  }
  transform(tasks);
}
