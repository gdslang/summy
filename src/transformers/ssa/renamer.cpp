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

#include <cppgdsl/rreil/rreil.h>
#include <cppgdsl/rreil/statement/statement_visitor.h>
#include <tuple>

using namespace std;
using namespace cfg;
namespace sr = summy::rreil;
using namespace gdsl::rreil;

void renamer::transform() {
  for(auto node : *cfg) {
    size_t node_id = node->get_id();
    auto &edges = *cfg->out_edges(node->get_id());
    vector<tuple<size_t, edge*>> updates;
    for(auto edge_it = edges.begin(); edge_it != edges.end(); edge_it++) {
      size_t edge_dst_node = edge_it->first;
      edge_visitor ev;

      sr::copy_visitor cv;
      auto rebuild = [&](auto &rd_elements, auto ast_node) {
        cv._([&](id *_id, int_t offset) -> variable* {
          shared_ptr<id> id_wrapped(_id, [](auto x) {});
          auto const &rd_mapping = rd_elements.find(id_wrapped);
          if(rd_mapping != rd_elements.end()) {
            _id = new sr::ssa_id(_id, rd_mapping->second);
          }
          return new variable(_id, offset);
        });
        ast_node->accept(cv);
      };

      auto &rd_dst_elements = rd_result.result[edge_dst_node]->get_elements();
      auto &rd_src_elements = rd_result.result[node_id]->get_elements();
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
          updates.push_back(make_tuple(edge_dst_node, new stmt_edge(&s)));
        });
        v._([&](load *l) {
          auto s = assignment(l->get_lhs(), l->get_address(), [&]() {
            return cv.get_address();
          }, [&](auto lhs, auto rhs) {
            return load(l->get_size(), lhs, rhs);
          });
          updates.push_back(make_tuple(edge_dst_node, new stmt_edge(&s)));
        });
        v._default([&](statement *s) {
          rebuild(rd_src_elements, s);
          auto stmt_new = cv.get_statement();
          updates.push_back(make_tuple(edge_dst_node, new stmt_edge(stmt_new)));
          delete stmt_new;
        });
        edge->get_stmt()->accept(v);
      });
      ev._([&](const cond_edge *ce) {
        rebuild(rd_src_elements, ce->get_cond());
        auto sexpr_new = cv.get_sexpr();
        updates.push_back(make_tuple(edge_dst_node, new cond_edge(sexpr_new, ce->is_positive())));
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
        updates.push_back(make_tuple(edge_dst_node, new phi_edge(assignments_new)));
      });
      edge_it->second->accept(ev);
    }
    for(auto &update : updates) {
      size_t dst_node;
      edge *edge;
      tie(dst_node, edge) = update;
      cfg->update_destroy_edge(node_id, dst_node, edge);
    }
  }
}
