/*
 * als_state.cpp
 *
 *  Created on: Mar 20, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/numeric/als_state.h>

using namespace analysis;
using namespace api;
using namespace std;

void als_state::put(std::ostream &out) const {
  bool first = true;
  out << "{";
  for(auto &elem : elements) {
    id_shared_t id = elem.first;
    id_set_t const &aliases = elem.second;
    if(!first)
      out << ' ';
    else
      first = false;
    out << "P(" << *id << ") -> ";
    for(auto &alias : aliases)
      out << *alias;
  }
  out << "Child state: {" << endl;
  out << *child_state;
  out << endl << "}";
}

bool als_state::is_bottom() const {
}

summy::vs_shared_t als_state::eval(api::num_linear* lin) {
}

summy::vs_shared_t als_state::eval(api::num_expr* exp) {
}

summy::vs_shared_t als_state::lookup(id_shared_t id) {
}

bool als_state::operator >=(const domain_state& other) const {
}

als_state* als_state::join(domain_state* other, size_t current_node) {
}

als_state* als_state::widen(domain_state* other, size_t current_node) {
}

als_state* als_state::narrow(domain_state* other, size_t current_node) {
}

als_state* als_state::box(domain_state* other, size_t current_node) {
}

void als_state::assign(api::num_var* lhs, api::num_expr* rhs) {
}

void als_state::assume(api::num_expr_cmp* cmp) {
}

void als_state::assume(api::num_var* lhs, anaylsis::api::ptr_set_t aliases) {
}

void als_state::kill(std::vector<api::num_var*> vars) {
}

void als_state::equate_kill(num_var_pairs_t vars) {
}

void als_state::fold(num_var_pairs_t vars) {
}

numeric_state* als_state::copy() {
}
