/*
 * memory_state.cpp
 *
 *  Created on: Mar 13, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/api/api.h>
#include <summy/analysis/domains/api/numeric/converter.h>
#include <summy/analysis/domains/memory_state.h>
#include <summy/rreil/shared_copy.h>
#include <cppgdsl/rreil/variable.h>
#include <string>
#include <sstream>
#include <tuple>

using namespace analysis::api;
using gdsl::rreil::variable;

using namespace analysis;
using namespace std;

void analysis::memory_state::put(std::ostream &out) const {
  auto print_fields = [&](id_shared_t mem_id, region_t region) {
    stringstream upper_boundary;
    stringstream field_line;
    stringstream offset_size_boundary;
    for(auto field_mapping : region) {
      size_t offset = field_mapping.first;
      size_t size = field_mapping.second.size;
      id_shared_t num_id = field_mapping.second.num_id;

      stringstream field_ss;
      field_ss << *num_id;
      string field_str = field_ss.str();

      stringstream pos_ss;
      pos_ss << offset << ":" << size;
      string pos_str = pos_ss.str();

      size_t field_size = std::max(field_str.length(), pos_str.length());
      /*
       * At least space before and the value set / two spaces after the offset:size
       */
      field_size += 2;
      /*
       * Field separators
       */
      field_size += 1;

      for(size_t i = 0; i < field_size; i++)
      upper_boundary << '-';

      field_line << "| " << field_str << ' ';
      for(size_t i = 0; i < field_size - field_str.length() - 3; i++)
      field_line << ' ';

      offset_size_boundary << pos_str << ' ';
      for(size_t i = 0; i < field_size - pos_str.length() - 1; i++)
      offset_size_boundary << '-';
    }

    upper_boundary << '-';
    field_line << '|';
    offset_size_boundary << '-';

    stringstream memory_id_ss;
    memory_id_ss << *mem_id;
    string memory_id_str = memory_id_ss.str();

    string sep = " -> ";

    for(size_t i = 0; i < memory_id_str.length() + sep.length(); i++)
    out << ' ';
    out << upper_boundary << endl;
    out << memory_id_str << sep << field_line << endl;
    for(size_t i = 0; i < memory_id_str.length() + sep.length(); i++)
    out << ' ';
    out << offset_size_boundary;
    out << endl;
  };

  out << "Regions: {" << endl;
  for(auto region_mapping : regions)
    print_fields(region_mapping.first, region_mapping.second);
  out << "}" << endl;
  out << "Deref: {" << endl;
  for(auto region_mapping : deref)
    print_fields(region_mapping.first, region_mapping.second);
  out << "}";
}

region_t &analysis::memory_state::region(id_shared_t id) {
  auto mapping_it = regions.find(id);
  if(mapping_it != regions.end()) return mapping_it->second;
  else {
    region_map_t::iterator ins_it;
    tie(ins_it, ignore) = regions.insert(std::make_pair(id, region_t()));
    return ins_it->second;
  }
}

std::tuple<id_shared_t, region_map_t, numeric_state*> analysis::memory_state::transVar(id_shared_t var_id,
    size_t offset, size_t size) {
  region_map_t regions = this->regions;
  auto &region = regions[var_id];
  auto field_it = region.find(offset);
  if(field_it == region.end()) {
    /*
     * Todo: this is broken ;-)
     */
    tie(field_it, ignore) = region.insert(make_pair(offset, field { size, var_id }));
  }
  field &f = field_it->second;
  if(f.size != size) throw string("analysis::memory_state::transVar(id_shared_t, size_t, size_t)");
  return make_tuple(f.num_id, regions, child_state);
}

bool analysis::memory_state::operator >=(const domain_state &other) const {
  throw string("analysis::memory_state::box(domain_state)");
}

memory_state *analysis::memory_state::join(domain_state *other, size_t current_node) {
  throw string("analysis::memory_state::box(domain_state,current_node)");
}

memory_state *analysis::memory_state::widen(domain_state *other, size_t current_node) {
  throw string("analysis::memory_state::box(domain_state,current_node)");
}

memory_state *analysis::memory_state::narrow(domain_state *other, size_t current_node) {
  throw string("analysis::memory_state::box(domain_state,current_node)");
}

memory_state *analysis::memory_state::box(domain_state *other, size_t current_node) {
  throw string("analysis::memory_state::box(domain_state,current_node)");
}

memory_state *analysis::memory_state::update(gdsl::rreil::assign *assign) {
  variable *var = assign->get_lhs();
  id_shared_t num_id;
  region_map_t regions_new;
  numeric_state *child_state_new;
  tie(num_id, regions_new, child_state_new) = transVar(shared_copy(var->get_id()), var->get_offset(), assign->get_size());
  num_var *n_var = new num_var(num_id);
  /*
   * Variables in rhs; converter needs transVar() as parameter
   */
  num_expr *n_expr = conv_expr(assign->get_rhs());
  child_state_new = child_state->assign(n_var, n_expr);
  return new memory_state(child_state_new, regions_new, deref);
}

