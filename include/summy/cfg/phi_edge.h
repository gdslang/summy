/*
 * phi_edge.h
 *
 *  Created on: Oct 17, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/cfg/edge.h>
#include <cppgdsl/rreil/variable.h>
extern "C" {
#include <gdsl_generic.h>
}
#include <vector>

namespace cfg {

class edge_visitor;

struct phi_assign {
  gdsl::rreil::variable *lhs;
  gdsl::rreil::variable *rhs;
  int_t size;

  phi_assign(gdsl::rreil::variable *lhs, gdsl::rreil::variable *rhs, int_t size) :
      lhs(lhs), rhs(rhs), size(size) {
  }
};

typedef std::vector<phi_assign> assignments_t;

class phi_edge: public edge {
private:
  assignments_t assignments;
public:
  phi_edge(assignments_t assignments);
  ~phi_edge();

  void dot(std::ostream &stream);
  virtual void accept(edge_visitor &v);
};

}
