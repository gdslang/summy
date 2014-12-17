/*
 * phi_edge.h
 *
 *  Created on: Oct 17, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/cfg/edge/edge.h>
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
  gdsl::rreil::variable *get_lhs() const {
    return lhs;
  }
  gdsl::rreil::variable *get_rhs() const {
    return rhs;
  }
  int_t get_size() const {
    return size;
  }

  phi_assign(gdsl::rreil::variable *lhs, gdsl::rreil::variable *rhs, int_t size);
  phi_assign(phi_assign const &a);
  ~phi_assign();
};

struct phi_memory {
  size_t from;
  size_t to;

  phi_memory(size_t from, size_t to) :
      from(from), to(to) {
  }
};

typedef std::vector<phi_assign> assignments_t;

class phi_edge: public edge {
private:
  assignments_t assignments;
  phi_memory memory;
public:
  phi_edge(assignments_t assignments, phi_memory memory);
  ~phi_edge();

  assignments_t const &get_assignments() const {
    return assignments;
  }

  phi_memory const &get_memory() const {
    return memory;
  }

  void dot(std::ostream &stream) const;
  virtual void accept(edge_visitor &v) const;
};

}
