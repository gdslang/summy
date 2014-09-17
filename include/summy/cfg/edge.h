/*
 * edge.h
 *
 *  Created on: Aug 19, 2014
 *      Author: jucs
 */

#pragma once

#include <stdint.h>
#include <iostream>
#include <cppgdsl/rreil/statement/statement.h>
#include <cppgdsl/rreil/sexpr/sexpr.h>
#include <functional>

namespace cfg {

class edge_visitor;

class edge {
private:
public:
  edge() {
  }
  virtual ~edge() {
  }

  virtual void dot(std::ostream &stream) = 0;
  virtual void accept(edge_visitor &v) = 0;
};

class stmt_edge: public edge {
private:
  gdsl::rreil::statement *stmt;
public:
  stmt_edge(gdsl::rreil::statement *stmt) :
      stmt(stmt) {
  }
  ~stmt_edge() {
  }

  gdsl::rreil::statement *get_stmt() {
    return stmt;
  }

  void dot(std::ostream &stream);
  virtual void accept(edge_visitor &v);
};

class cond_edge: public edge {
private:
  gdsl::rreil::sexpr *cond;
  bool positive;
public:
  cond_edge(gdsl::rreil::sexpr *cond, bool positive) :
      cond(cond), positive(positive) {
  }
  ~cond_edge() {
  }

  gdsl::rreil::sexpr *get_cond() {
    return cond;
  }

  void dot(std::ostream &stream);
  virtual void accept(edge_visitor &v);
};

class edge_visitor {
private:
  std::function<void(stmt_edge*)> stmt_edge_callback = NULL;
  std::function<void(cond_edge*)> cond_edge_callback = NULL;

public:
  virtual ~edge_visitor() {
  }

  virtual void visit(stmt_edge *se) {
    if(stmt_edge_callback != NULL)
      stmt_edge_callback(se);
    _default();
  }

  virtual void visit(cond_edge *se) {
    if(cond_edge_callback != NULL)
      cond_edge_callback(se);
    _default();
  }

  virtual void _default() {
  }

  void _(std::function<void(stmt_edge*)> stmt_edge_callback) {
    this->stmt_edge_callback = stmt_edge_callback;
  }

  void _(std::function<void(cond_edge*)> cond_edge_callback) {
    this->cond_edge_callback = cond_edge_callback;
  }
};

}
