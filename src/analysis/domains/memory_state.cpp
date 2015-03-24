/*
 * memory_state.cpp
 *
 *  Created on: Mar 13, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/api/api.h>
#include <summy/analysis/domains/api/numeric/converter.h>
#include <summy/analysis/domains/memory_state.h>
#include <summy/rreil/id/numeric_id.h>
#include <summy/rreil/shared_copy.h>
#include <summy/value_set/value_set.h>
#include <summy/value_set/vs_finite.h>
#include <cppgdsl/rreil/variable.h>
#include <cppgdsl/rreil/id/id.h>
#include <summy/rreil/id/memory_id.h>
#include <algorithm>
#include <iosfwd>
#include <string>
#include <sstream>
#include <tuple>
#include <vector>

using namespace analysis::api;
using namespace summy;
using namespace gdsl::rreil;
using summy::rreil::memory_id;
using summy::rreil::numeric_id;

using namespace analysis;
using namespace std;

analysis::memory_state::temp_s::~temp_s() {
  _this.child_state->kill( { var });
  delete var;
}

memory_state::temp_s analysis::memory_state::assign_address(address *a) {
  num_var *var = new num_var(numeric_id::generate());
  converter addr_cv([&](shared_ptr<gdsl::rreil::id> id, size_t offset) {
    return transLE(id, offset, a->get_size());
  });
  num_expr *addr_expr = addr_cv.conv_expr(a->get_lin());
  child_state->assign(var, addr_expr);
  delete addr_expr;

  return temp_s { *this, var };
}

region_t &analysis::memory_state::dereference(id_shared_t id) {
  auto id_it = deref.find(id);
  if(id_it == deref.end()) tie(id_it, ignore) = deref.insert(make_pair(id, region_t { }));
  return id_it->second;
}

void analysis::memory_state::put(std::ostream &out) const {
  auto print_fields = [&](bool deref, id_shared_t mem_id, region_t region) {
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
    memory_id_ss << (deref ? "*" : "") << *mem_id;
    string memory_id_str = memory_id_ss.str();

    string sep = " -> ";

    for(size_t i = 0; i < memory_id_str.length() + sep.length(); i++)
    out << ' ';
    out << upper_boundary.str() << endl;
    out << memory_id_str << sep << field_line.str() << endl;
    for(size_t i = 0; i < memory_id_str.length() + sep.length(); i++)
    out << ' ';
    out << offset_size_boundary.str();
    out << endl;
  };

  out << "Regions: {" << endl;
  for(auto region_mapping : regions)
    print_fields(false, region_mapping.first, region_mapping.second);
  out << "}" << endl;
  out << "Deref: {" << endl;
  for(auto region_mapping : deref)
    print_fields(true, region_mapping.first, region_mapping.second);
  out << "}" << endl;
  out << "Child state: {" << endl;
  out << *child_state;
  out << endl << "}";
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

static bool overlap(size_t from_a, size_t size_a, size_t from_b, size_t size_b) {
//  cout << "overlap(" << from_a << ", " << size_a << ", " << from_b << ", " << size_b << endl;
  if(!size_a || !size_b) return false;
  size_t to_a = from_a + size_a - 1;
  size_t to_b = from_b + size_b - 1;
  if(from_a < from_b) {
    return to_a >= from_b;
  } else if(from_a == from_b) return true;
  else return to_b >= from_a;
}

id_shared_t analysis::memory_state::transVar(id_shared_t var_id, size_t offset, size_t size) {
  auto &region = regions[var_id];
  bool found = false;
  id_shared_t num_id;
  vector<num_var*> dead_num_vars;
  auto field_it = region.upper_bound(offset);
  if(field_it != region.begin()) --field_it;
  while(field_it != region.end()) {
    bool erase = false;
    size_t offset_next = field_it->first;
    field &f_next = field_it->second;
//    cout << "Considering " << offset_next << " / " << f_next.size << endl;
    if(offset_next == offset && f_next.size == size) {
      found = true;
      num_id = f_next.num_id;
      break;
    } else if(overlap(offset_next, f_next.size, offset, size)) {
      dead_num_vars.push_back(new num_var(f_next.num_id));
      erase = true;
    }
    if(offset_next >= offset + size) break;
    if(erase) region.erase(field_it++);
    else field_it++;
  }
  if(!found) tie(field_it, ignore) = region.insert(make_pair(offset, field { size, numeric_id::generate() }));
  field &f = field_it->second;
  child_state->kill(dead_num_vars);
  for(auto var : dead_num_vars)
    delete var;
  return f.num_id;
}

num_linear *analysis::memory_state::transLE(id_shared_t var_id, size_t offset, size_t size) {
  auto &region = regions[var_id];
  auto field_it = region.find(offset);
  if(field_it == region.end()) tie(field_it, ignore) = region.insert(
      make_pair(0, field { size, numeric_id::generate() }));
  field &f = field_it->second;
  if(f.size == size) return new num_linear_term(new num_var(f.num_id));
  return new num_linear_vs(value_set::top);
}

analysis::memory_state::memory_state(numeric_state *child_state, bool start_bottom) :
    child_state(child_state) {
  auto arch_ptr = [&](string id_name) {
    num_var *nv = new num_var(shared_ptr<gdsl::rreil::id>(new arch_id(id_name)));

    field f = field {64, numeric_id::generate()};
    region_t r = region_t {make_pair(0, f)};
    regions.insert(make_pair(nv->get_id(), r));

    num_var *ptr_var = new num_var(f.num_id);
    child_state->assume(ptr_var, {ptr(shared_ptr<gdsl::rreil::id>(new memory_id(0, nv->get_id())), vs_finite::zero)});
    delete nv;
    delete ptr_var;
  };
  if(start_bottom) {
    /*
     * start value
     */
    arch_ptr("IP");
    arch_ptr("SP");
    arch_ptr("A");
    arch_ptr("B");
    arch_ptr("C");
    arch_ptr("D");
  } else {
    /*
     * bottom
     */

  }
}

bool analysis::memory_state::is_bottom() {
  return child_state->is_bottom();
}

bool analysis::memory_state::operator >=(const domain_state &other) const {
  memory_state const &other_casted = dynamic_cast<memory_state const&>(other);
  numeric_state *me_compat;
  numeric_state *other_compat;
  tie(ignore, me_compat, other_compat) = compat(this, &other_casted);
  bool result = *me_compat >= *other_compat;
  delete me_compat;
  delete other_compat;
  return result;
}

memory_state *analysis::memory_state::join(domain_state *other, size_t current_node) {
  memory_state *other_casted = dynamic_cast<memory_state *>(other);
  numeric_state *me_compat;
  numeric_state *other_compat;
  memory_head head_compat;
  tie(head_compat, me_compat, other_compat) = compat(this, other_casted);
  return new memory_state(me_compat->join(other_compat, current_node), head_compat.regions, head_compat.deref);
}

memory_state *analysis::memory_state::widen(domain_state *other, size_t current_node) {
  throw string("analysis::memory_state::box(domain_state,current_node)");
}

memory_state *analysis::memory_state::narrow(domain_state *other, size_t current_node) {
  throw string("analysis::memory_state::box(domain_state,current_node)");
}

memory_state *analysis::memory_state::box(domain_state *other, size_t current_node) {
  memory_state *other_casted = dynamic_cast<memory_state *>(other);
  numeric_state *me_compat;
  numeric_state *other_compat;
  memory_head head_compat;
  tie(head_compat, me_compat, other_compat) = compat(this, other_casted);
  return new memory_state(me_compat->box(other_compat, current_node), head_compat.regions, head_compat.deref);
}

void analysis::memory_state::update(gdsl::rreil::assign *assign) {
  variable *var = assign->get_lhs();
  id_shared_t num_id = transVar(shared_copy(var->get_id()), var->get_offset(), assign->get_size());
  num_var *n_var = new num_var(num_id);
  /*
   * Variables in rhs; converter needs transVar() as parameter
   */
  converter cv([&](shared_ptr<gdsl::rreil::id> id, size_t offset) {
    return transLE(id, offset, assign->get_size());
  });
  num_expr *n_expr = cv.conv_expr(assign->get_rhs());
  child_state->assign(n_var, n_expr);
  delete n_expr;
  delete n_var;
}

memory_state *analysis::memory_state::copy() const {
  return new memory_state(*this);
}

memory_state *analysis::memory_state::start_value(numeric_state *start_num) {
  return new memory_state(start_num, true);
}

void analysis::memory_state::update(gdsl::rreil::load *load) {
  auto temp = assign_address(load->get_address());

  ptr_set_t aliases = child_state->queryAls(temp.var);
  for(auto &alias : aliases) {
    region_t &region = dereference(alias.id);
    auto zero_it = region.find(0);
    if(region.find(0) == region.end()) tie(zero_it, ignore) = region.insert(make_pair(0, field { 64,
        numeric_id::generate() }));
    num_var *rhs = new num_var(zero_it->second.num_id);
    num_expr *rhs_expr = new num_expr_lin(new num_linear_term(rhs));
    num_var *lhs = new num_var(
        transVar(shared_copy(load->get_lhs()->get_id()), load->get_lhs()->get_offset(), load->get_size()));
    child_state->assign(lhs, rhs_expr);

    delete rhs_expr;
    delete lhs;
  }
}

void analysis::memory_state::update(gdsl::rreil::store *store) {
  auto temp = assign_address(store->get_address());
  ;

  ptr_set_t aliases = child_state->queryAls(temp.var);
  for(auto &alias : aliases) {
    region_t &region = dereference(alias.id);
    auto zero_it = region.find(0);
    if(region.find(0) == region.end()) tie(zero_it, ignore) = region.insert(make_pair(0, field { 64,
        numeric_id::generate() }));
    num_var *lhs = new num_var(zero_it->second.num_id);

    converter rhs_cv([&](shared_ptr<gdsl::rreil::id> id, size_t offset) {
      return transLE(id, offset, store->get_size());
    });
    num_expr *rhs = rhs_cv.conv_expr(store->get_rhs());
    child_state->assign(lhs, rhs);

    delete lhs;
    delete rhs;
  }
}

memory_state *analysis::memory_state::bottom(numeric_state *bottom_num) {
  return new memory_state(bottom_num, false);
}

std::tuple<memory_state::memory_head, numeric_state*, numeric_state*> analysis::memory_state::compat(
    const memory_state *a, const memory_state *b) {
  numeric_state *a_n = a->child_state->copy();
  numeric_state *b_n = b->child_state->copy();

  auto join_region_map = [&](region_map_t const &a_map, region_map_t const &b_map) {
    region_map_t result_map;

    auto handle_region = [&](id_shared_t id, region_t const &region_a, region_t const &region_b) {
      num_var_pairs_t equate_kill_vars;
      for(auto &field_it : region_a) {
        auto field_b_it = region_b.find(field_it.first);
        if(field_b_it != region_b.end()) {
          field const &f = field_it.second;
          field const &f_b = field_b_it->second;
          if(!(*f.num_id == *f_b.num_id))
            equate_kill_vars.push_back(make_tuple(new num_var(f.num_id), new num_var(f_b.num_id)));
        }
      }
      b_n->equate_kill(equate_kill_vars);
      for(auto pair : equate_kill_vars) {
        num_var *a, *b;
        tie(a, b) = pair;
        delete a;
        delete b;
      }

      region_map_t::iterator head_region_it;
      tie(head_region_it, ignore) = result_map.insert(make_pair(id, region_t()));
      region_t &region = head_region_it->second;

      auto join = [&](numeric_state *n, region_t const &from, region_t const &to) {
        auto kill = [&](id_shared_t id) {
          num_var *nv = new num_var(id);
          n->kill({nv});
          delete nv;
        };

        for(auto &field_it : from) {
          field const &f = field_it.second;
          auto field_b_it = to.find(field_it.first);
          if(field_b_it != to.end()) {
            field const &f_b = field_b_it->second;
            if(f.size == f_b.size)
              region.insert(make_pair(field_it.first, field {f.size, f.num_id}));
            else
              kill(f.num_id);
          } else
            kill(f.num_id);
        }
      };

      join(a_n, region_a, region_b);
      join(b_n, region_b, region_a);
    };
    for(auto &region_it : a_map) {
      auto region_b_it = b_map.find(region_it.first);
      if(region_b_it != b_map.end())
        handle_region(region_it.first, region_it.second, region_b_it->second);
      else
        handle_region(region_it.first, region_it.second, region_t());
    }
    for(auto &region_b_it : b_map) {
      if(a_map.find(region_b_it.first) == a_map.end())
        handle_region(region_b_it.first, region_b_it.second, region_t());
    }

    return result_map;
  };

  memory_head head;
  head.regions = join_region_map(a->regions, b->regions);
  head.deref = join_region_map(a->deref, b->deref);

  return make_tuple(head, a_n, b_n);
}
