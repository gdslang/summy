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

class phi_assign {
private:
  gdsl::rreil::variable *lhs;
  gdsl::rreil::variable *rhs;
  int_t size;

public:
  gdsl::rreil::variable *get_lhs() {
    return lhs;
  }
  gdsl::rreil::variable *get_rhs() {
    return rhs;
  }
  int_t get_size() {
    return size;
  }

  phi_assign(gdsl::rreil::variable *lhs, gdsl::rreil::variable *rhs, int_t size);
  phi_assign(phi_assign const &a);
  ~phi_assign();
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
