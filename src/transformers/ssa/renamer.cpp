/*
 * renamer.cpp
 *
 *  Created on: Oct 20, 2014
 *      Author: Julian Kranz
 */

#include <summy/transformers/ssa/renamer.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/edge.h>
#include <summy/cfg/edge_visitor.h>
#include <summy/cfg/phi_edge.h>
#include <summy/rreil/id/ssa_id.h>
#include <summy/rreil/copy_visitor.h>
#include <summy/rreil/visitor.h>
#include <summy/rreil/id/id_visitor.h>

#include <cppgdsl/rreil/rreil.h>
#include <cppgdsl/rreil/statement/statement_visitor.h>
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
  auto rebuild = [&](auto &rd_elements, auto ast_node) {
    cv._([&](id *_id, int_t offset) -> variable* {
      shared_ptr<id> id_wrapped(_id, [](auto x) {});
      auto const &rd_mapping = rd_elements.find(id_wrapped);
      if(rd_mapping != rd_elements.end()) {
        sr::id_visitor iv;
        iv._([&](sr::ssa_id *si) {
          if(si->get_version() != rd_mapping->second) {
            sr::copy_visitor cv;
            si->get_id()->accept(cv);
            delete _id;
            _id = new sr::ssa_id(cv.get_id(), rd_mapping->second);
            update = true;
          }
        });
        iv._default([&](id *_) {
            _id = new sr::ssa_id(_id, rd_mapping->second);
            update = true;
        });
        _id->accept(iv);
      }
      return new variable(_id, offset);
    });
    ast_node->accept(cv);
  };

  auto &rd_dst_elements = rd_result.result[to]->get_elements();
  auto &rd_src_elements = rd_result.result[from]->get_elements();
  auto assignment = [&](variable *lhs, auto rhs, auto get_rhs_from_cv, auto node_ctor) {
    rebuild(rd_dst_elements, lhs);
    variable *lhs_new = cv.get_variable();

    rebuild(rd_src_elements, rhs);
    auto rhs_new = get_rhs_from_cv();

    return node_ctor(lhs_new, rhs_new);
  };

  ev._([&](const stmt_edge *edge) {
    statement_visitor v;
    v._([&](assign *a) {
      auto s = assignment(a->get_lhs(), a->get_rhs(), [&]() {
        return cv.get_expr();
      }, [&](auto lhs, auto rhs) {
        return assign(a->get_size(), lhs, rhs);
      });
      add_update(from, to, new stmt_edge(&s));
    });
    v._([&](load *l) {
      auto s = assignment(l->get_lhs(), l->get_address(), [&]() {
        return cv.get_address();
      }, [&](auto lhs, auto rhs) {
        return load(l->get_size(), lhs, rhs);
      });
      add_update(from, to, new stmt_edge(&s));
    });
    v._default([&](statement *s) {
      rebuild(rd_src_elements, s);
      auto stmt_new = cv.get_statement();
      add_update(from, to, new stmt_edge(stmt_new));
      delete stmt_new;
    });
    edge->get_stmt()->accept(v);
  });
  ev._([&](const cond_edge *ce) {
    rebuild(rd_src_elements, ce->get_cond());
    auto sexpr_new = cv.get_sexpr();
    add_update(from, to, new cond_edge(sexpr_new, ce->is_positive()));
    delete sexpr_new;
  });
  ev._([&](const phi_edge *edge) {
    assignments_t assignments_new;
    for(auto &ass : edge->get_assignments()) {
      auto ass_new = assignment(ass.get_lhs(), ass.get_rhs(), [&]() {
        return cv.get_variable();
      }, [&](auto lhs, auto rhs) {
        auto phi_ass = phi_assign(lhs, rhs, ass.get_size());
        delete lhs;
        delete rhs;
        return phi_ass;
      });
      assignments_new.push_back(ass_new);
    }
    add_update(from, to, new phi_edge(assignments_new));
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
    auto &edges = *cfg->out_edges(node->get_id());
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
    task_from_edge(tasks, from, to, cfg->out_edges(from)->at(to));
  }
  transform(tasks);
}
