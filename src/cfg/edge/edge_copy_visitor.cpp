/*
 * edge_copy_visitor.cpp
 *
 *  Created on: Oct 29, 2014
 *      Author: Julian Kranz
 */

#include <summy/cfg/edge/edge_copy_visitor.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/phi_edge.h>
//#include <summy/rreil/copy_visitor.h>

void cfg::edge_copy_visitor::visit(const edge *e) {
  if(edge_ctor != NULL) _edge = edge_ctor();
  else {
    _edge = new edge();
    _default();
  }
}

void cfg::edge_copy_visitor::visit(const stmt_edge *e) {
  if(stmt_edge_ctor != NULL) _edge = stmt_edge_ctor(e->get_stmt());
  else {
    _edge = new stmt_edge(e->get_stmt());
    _default();
  }
}

void cfg::edge_copy_visitor::visit(const cond_edge *e) {
  if(cond_edge_ctor != NULL) _edge = cond_edge_ctor(e->get_cond(), e->is_positive());
  else {
    _edge = new cond_edge(e->get_cond(), e->is_positive());
    _default();
  }
}

void cfg::edge_copy_visitor::visit(const phi_edge *e) {
  if(cond_edge_ctor != NULL) _edge = phi_edge_ctor(e->get_assignments());
  else {
    _edge = new phi_edge(e->get_assignments());
    _default();
  }
}

void cfg::edge_copy_visitor::_default() {
  if(default_callback != NULL) default_callback();
}
