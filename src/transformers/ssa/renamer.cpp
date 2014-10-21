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
        bool update = false;
        cv._([&](id *_id, int_t offset) -> variable* {
          shared_ptr<id> id_wrapped(_id, [](auto x) {});
          auto const &rd_mapping = rd_elements.find(id_wrapped);
          if(rd_mapping != rd_elements.end()) {
            _id = new sr::ssa_id(_id, rd_mapping->second);
            update = true;
          }
          return new variable(_id, offset);
        });
        ast_node->accept(cv);
        return update;
      };

      auto assignment = [&](variable *var, auto rhs, auto get_rhs, auto ctor_node) {
        auto &rd_dst_elements = rd_result->result[edge_dst_node]->get_elements();
        auto update = rebuild(rd_dst_elements, var);
        variable *lhs = cv.get_variable();

        if(update) {
          auto &rd_src_elements = rd_result->result[node_id]->get_elements();
          rebuild(rd_src_elements, rhs);
          auto rhs = get_rhs();

          auto r = ctor_node(lhs, rhs);
          updates.push_back(make_tuple(edge_dst_node, new stmt_edge(&r)));
        } else
          delete lhs;
      };

      ev._([&](stmt_edge *edge) {
        statement_visitor v;
        v._([&](assign *a) {
          assignment(a->get_lhs(), a->get_rhs(), [&]() {
            return cv.get_expr();
          }, [&](auto lhs, auto rhs) {
            return assign(a->get_size(), lhs, rhs);
          });
        });
        v._([&](load *l) {
          assignment(l->get_lhs(), l->get_address(), [&]() {
            return cv.get_address();
          }, [&](auto lhs, auto rhs) {
            return load(l->get_size(), lhs, rhs);
          });
        });

        edge->get_stmt()->accept(v);
      });
      edge_it->second->accept(ev);
    }
    for(auto &update : updates) {
      size_t dst_node;
      edge *stmt_edge;
      tie(dst_node, stmt_edge) = update;
      edges[dst_node] = stmt_edge;
    }
  }
}
