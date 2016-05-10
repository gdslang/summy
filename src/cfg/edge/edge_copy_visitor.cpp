/*
 * edge_copy_visitor.cpp
 *
 *  Created on: Oct 29, 2014
 *      Author: Julian Kranz
 */

#include <summy/cfg/edge/edge_copy_visitor.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/edge/phi_edge.h>
//#include <summy/rreil/copy_visitor.h>

void cfg::edge_copy_visitor::visit(const edge *e) {
  if(edge_ctor != NULL) _edge = edge_ctor(e->get_jump_dir());
  else {
    _edge = new edge(e->get_jump_dir());
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
  if(cond_edge_ctor != NULL) _edge = phi_edge_ctor(e->get_assignments(), e->get_memory());
  else {
    _edge = new phi_edge(e->get_assignments(), e->get_memory());
    _default();
  }
}

void cfg::edge_copy_visitor::visit(const call_edge *e) {
  if(call_edge_ctor != NULL) _edge = call_edge_ctor(e->is_target_edge());
  else {
    _edge = new call_edge(e->is_target_edge());
    _default();
  }
}

void cfg::edge_copy_visitor::_default() {
  if(default_callback != NULL) default_callback();
}
