/*
 * edge_copy_visitor.h
 *
 *  Created on: Oct 29, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/cfg/edge/edge_visitor.h>
#include <summy/cfg/edge/phi_edge.h>
#include <cppgdsl/rreil/statement/statement.h>
#include <cppgdsl/rreil/sexpr/sexpr.h>
#include <functional>

namespace cfg {

class edge;
class stmt_edge;
class cond_edge;

class edge_copy_visitor : public edge_visitor {
public:
  typedef std::function<edge*()> edge_ctor_t;
  typedef std::function<stmt_edge*(gdsl::rreil::statement *stmt)> stmt_edge_ctor_t;
  typedef std::function<cond_edge*(gdsl::rreil::sexpr *cond, bool positive)> cond_edge_ctor_t;
  typedef std::function<phi_edge*(assignments_t assignments, phi_memory memory)> phi_edge_ctor_t;
  typedef std::function<void(void)> default_callback_t;
private:
  edge_ctor_t edge_ctor = NULL;
  stmt_edge_ctor_t stmt_edge_ctor = NULL;
  cond_edge_ctor_t cond_edge_ctor = NULL;
  phi_edge_ctor_t phi_edge_ctor = NULL;
  default_callback_t default_callback = NULL;
protected:
  union {
    edge *_edge;
  };
public:
  edge *get_edge() {
    return _edge;
  }

  virtual void visit(const edge *e);
  virtual void visit(const stmt_edge *e);
  virtual void visit(const cond_edge *e);
  virtual void visit(const phi_edge *e);
  virtual void _default();

  void _(edge_ctor_t c) {
    this->edge_ctor = c;
  }
  void _(stmt_edge_ctor_t c) {
    this->stmt_edge_ctor = c;
  }
  void _(cond_edge_ctor_t c) {
    this->cond_edge_ctor = c;
  }
  void _(phi_edge_ctor_t c) {
    this->phi_edge_ctor = c;
  }
  void _default(default_callback_t c) {
    this->default_callback = c;
  }
};
}
