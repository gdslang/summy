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

  virtual void dot(std::ostream &stream);
  virtual void accept(edge_visitor &v);
};

class stmt_edge: public edge {
private:
  gdsl::rreil::statement *stmt;
public:
  stmt_edge(gdsl::rreil::statement *stmt);
  ~stmt_edge() {
    delete stmt;
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
  cond_edge(gdsl::rreil::sexpr *cond, bool positive);
  ~cond_edge() {
    delete cond;
  }

  gdsl::rreil::sexpr *get_cond() {
    return cond;
  }

  void dot(std::ostream &stream);
  virtual void accept(edge_visitor &v);
};

}
