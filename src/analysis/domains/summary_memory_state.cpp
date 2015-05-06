/*
 * summary_memory_state.cpp
 *
 *  Created on: Mar 13, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/api/api.h>
#include <summy/analysis/domains/api/numeric/converter.h>
#include <summy/analysis/domains/summary_memory_state.h>
#include <summy/rreil/id/numeric_id.h>
#include <summy/rreil/id/sm_id.h>
#include <summy/rreil/shared_copy.h>
#include <summy/value_set/value_set.h>
#include <summy/value_set/vs_finite.h>
#include <summy/value_set/value_set_visitor.h>
#include <cppgdsl/rreil/variable.h>
#include <cppgdsl/rreil/id/id.h>
#include <include/summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/memory_id.h>
#include <algorithm>
#include <iosfwd>
#include <string>
#include <sstream>
#include <tuple>
#include <vector>
#include <assert.h>

using namespace analysis::api;
using namespace summy;
using namespace gdsl::rreil;
using namespace summy::rreil;


using namespace analysis;
using namespace std;

/*
 * managed temporary
 */

analysis::managed_temporary::managed_temporary(summary_memory_state &_this, api::num_var *var) :
    _this(_this), var(var) {
}

managed_temporary::~managed_temporary() {
  _this.child_state->kill( { var });
  delete var;
}

/*
 * field & relation
 */

std::ostream& analysis::operator <<(std::ostream &out, const field &_this) {
  out << "{" << *_this.num_id << ":" << _this.size << "}";
  return out;
}


void analysis::relation::clear() {
  regions.clear();
  deref.clear();
}

field &analysis::io_region::insert(numeric_state *child_state, int64_t offset, size_t size) {
  id_shared_t nid_in = numeric_id::generate();
  id_shared_t nid_out = numeric_id::generate();

  num_var *n_in = new num_var(nid_in);
  num_var *n_out = new num_var(nid_out);
//  num_expr_cmp *in_out_eq = num_expr_cmp::equals(n_in, n_out);
  num_expr *ass_e = new num_expr_lin(new num_linear_term(n_in));

  child_state->assume(n_in, {ptr(shared_ptr<gdsl::rreil::id>(new memory_id(0, nid_in)), vs_finite::zero)});
//  child_state->assume(in_out_eq);
  child_state->assign(n_out, ass_e);

  in_r.insert(make_pair(offset, field { size, nid_in }));
  region_t::iterator field_out_it;
  tie(field_out_it, ignore) = out_r.insert(make_pair(offset, field { size, nid_out }));

  delete n_out;
  delete ass_e;

  return field_out_it->second;
}

/*
 * summary memory state
 */

summary_memory_state *analysis::summary_memory_state::domop(domain_state *other, size_t current_node, domopper_t domopper) {
  //  cout << "JOIN OF" << endl;
  //  cout << *this << endl;
  //  cout << "WITH" << endl;
  //  cout << *other << endl;
    summary_memory_state *other_casted = dynamic_cast<summary_memory_state *>(other);
    if(is_bottom()) return other_casted->copy();
    else if(other_casted->is_bottom()) return copy();

    numeric_state *me_compat;
    numeric_state *other_compat;
    memory_head head_compat;
    tie(head_compat, me_compat, other_compat) = compat(this, other_casted);

  //  cout << *me_compat << " ^^^JOIN^^^ " << *other_compat << endl;

    summary_memory_state *result = new summary_memory_state(sm, (me_compat->*domopper)(other_compat, current_node), head_compat.input,
        head_compat.output);
    delete me_compat;
    delete other_compat;
    result->cleanup();
    return result;
}

std::unique_ptr<managed_temporary> analysis::summary_memory_state::assign_temporary(int_t size,
    std::function<num_expr*(analysis::api::converter&)> cvc) {
  num_var *var = new num_var(numeric_id::generate());
  converter addr_cv(size, [&](shared_ptr<gdsl::rreil::id> id, size_t offset, size_t size) {
    return transLE(id, offset, size);
  });
  num_expr *addr_expr = cvc(addr_cv);
  child_state->assign(var, addr_expr);
  delete addr_expr;

  return unique_ptr<managed_temporary>(new managed_temporary(*this, var));
}

io_region analysis::summary_memory_state::region_by_id(region_map_t&(relation::*getter)(), id_shared_t id) {
  region_map_t &input_rmap =  (input.*getter)();
  region_map_t &output_rmap =  (output.*getter)();
  auto id_in_it = input_rmap.find(id);
  if(id_in_it == input_rmap.end())
    tie(id_in_it, ignore) = input_rmap.insert(make_pair(id, region_t { }));
  auto id_out_it = output_rmap.find(id);
  if(id_out_it == output_rmap.end())
    tie(id_out_it, ignore) = output_rmap.insert(make_pair(id, region_t { }));
  return io_region { id_in_it->second, id_out_it->second };
}

void analysis::summary_memory_state::bottomify() {
  child_state->bottomify();
  input.clear();
  output.clear();
}

io_region analysis::summary_memory_state::dereference(id_shared_t id) {
  return region_by_id(&relation::get_deref, id);
}

void analysis::summary_memory_state::put(std::ostream &out) const {
  auto print_fields = [&](bool deref, id_shared_t mem_id, region_t region) {
    stringstream upper_boundary;
    stringstream field_line;
    stringstream offset_size_boundary;
    for(auto field_mapping : region) {
      int64_t offset = field_mapping.first;
      size_t size = field_mapping.second.size;
      id_shared_t num_id = field_mapping.second.num_id;

      stringstream field_ss;
      field_ss << *num_id;
      string field_str = field_ss.str();

      stringstream pos_ss;
      pos_ss << (offset < 0 ? "(" : "") << offset << (offset < 0 ? ")" : "") << ":" << size;
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

  out << "Regions input: {" << endl;
  for(auto region_mapping : input.regions)
    print_fields(false, region_mapping.first, region_mapping.second);
  out << "}" << endl;
  out << "Deref input: {" << endl;
  for(auto region_mapping : input.deref)
    print_fields(true, region_mapping.first, region_mapping.second);
  out << "}" << endl;

  out << "Regions ouptut: {" << endl;
  for(auto region_mapping : output.regions)
    print_fields(false, region_mapping.first, region_mapping.second);
  out << "}" << endl;
  out << "Deref output: {" << endl;
  for(auto region_mapping : output.deref)
    print_fields(true, region_mapping.first, region_mapping.second);
  out << "}" << endl;

  out << "Child state: {" << endl;
  out << *child_state;
  out << endl << "}";
}

//region_t &analysis::summary_memory_state::region(id_shared_t id) {
//  auto mapping_it = regions.find(id);
//  if(mapping_it != regions.end()) return mapping_it->second;
//  else {
//    region_map_t::iterator ins_it;
//    tie(ins_it, ignore) = regions.insert(std::make_pair(id, region_t()));
//    return ins_it->second;
//  }
//}

tuple<bool, void*> analysis::summary_memory_state::static_address(id_shared_t id) {
  bool is_static = false;
  void *symbol_address;
  summy::rreil::id_visitor idv;
  idv._([&](sm_id *sid) {
    is_static = true;
    symbol_address = sid->get_address();
  });
  id->accept(idv);
  return make_tuple(is_static, symbol_address);
}

void analysis::summary_memory_state::initialize_static(io_region io, void *address, size_t offset, size_t size) {
  id_shared_t mem_id = transVarReg(io, offset, size);
  if(size > 64)
    throw string("analysis::summary_memory_state::initialize_static(region_t,void*,size_t): size > 64");

  int64_t sv = 0;
  bool success = sm->read((char*)address + (offset/8), size/8, (uint8_t*)&sv);
//  cout << "read " << (size_t)address << "/" << + (offset/8) << " " << size/8 << " " << sv << " " << success << endl;
  if(success) {
    vs_shared_t sv_vs = vs_finite::single(sv);
    num_var *v_mem_id = new num_var(mem_id);
    num_expr *e_sv_vs = new num_expr_lin(new num_linear_vs(sv_vs));
    child_state->assign(v_mem_id, e_sv_vs);
    delete e_sv_vs;
    delete v_mem_id;
  }
}

std::tuple<std::set<int64_t>, std::set<int64_t> > analysis::summary_memory_state::overlappings(summy::vs_finite *vs,
    int_t store_size) {
  set<int64_t> overlapping;
  set<int64_t> non_overlapping;

  vs_finite::elements_t const &v_elems = vs->get_elements();

  int64_t last;
  for(auto o_it = v_elems.begin(); o_it != v_elems.end(); o_it++) {
    int64_t next = *o_it;
    if(o_it != v_elems.begin()) {
      if(next >= last + store_size)
        non_overlapping.insert(last);
      else {
        overlapping.insert(last);
        overlapping.insert(next);
      }
    }
    last = next;
  }
  if(overlapping.find(last) == overlapping.end())
    non_overlapping.insert(last);

  return make_tuple(overlapping, non_overlapping);
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

bool analysis::summary_memory_state::overlap_region(region_t& region, int64_t offset, size_t size) {
  /*
   * Todo: uncopy from retrieve_kill()?
   */
  bool _overlap = false;

  auto field_it = region.upper_bound(offset);
  if(field_it != region.begin()) --field_it;
  while(field_it != region.end()) {
    int64_t offset_next = field_it->first;
    field &f_next = field_it->second;
    if(overlap(offset_next, f_next.size, offset, size))
      _overlap = true;
    if(offset_next >= offset + size) break;
    else field_it++;
  }

  return _overlap;
}

/*
 * Todo: Work on io_region, keep more relations between input and output
 */
region_t::iterator analysis::summary_memory_state::retrieve_kill(region_t &region, int64_t offset,
    size_t size) {
//  cout << "retrieve_kill() " << offset << " / " << size << endl;

  bool found = false;
  id_shared_t num_id;
  vector<num_var*> dead_num_vars;
  vector<tuple<int64_t, size_t>> replacements;
  auto field_it = region.upper_bound(offset);

//  if(field_it == region.end())
//    cout << "Upper bound is region end" << endl;
//  else
//    cout << "Upper bound: " << field_it->second;

  if(field_it != region.begin()) --field_it;
  while(field_it != region.end()) {
    bool erase = false;
    int64_t offset_next = field_it->first;
    field &f_next = field_it->second;
//    cout << "Considering " << offset_next << " / " << f_next.size << endl;
    if(offset_next == offset && f_next.size == size) {
      found = true;
      num_id = f_next.num_id;
      break;
    } else if(overlap(offset_next, f_next.size, offset, size)) {
      if(offset_next < offset) {
        size_t first_size = offset - offset_next + 1;
        replacements.push_back(make_tuple(offset_next, first_size));
        if(f_next.size > size + first_size)
          replacements.push_back(make_tuple(offset + size, f_next.size - size - first_size));
      } else
        replacements.push_back(make_tuple(offset + size, f_next.size - size));

      dead_num_vars.push_back(new num_var(f_next.num_id));
      erase = true;
    }
    if(offset_next >= offset + size) break;
    if(erase) region.erase(field_it++);
    else field_it++;
  }
  child_state->kill(dead_num_vars);
  for(auto var : dead_num_vars)
    delete var;
  for(auto repl : replacements) {
    int64_t offset;
    size_t size;
    tie(offset, size) = repl;
    region.insert(make_pair(offset, field { size, numeric_id::generate() }));
  }

//  cout << "Found: " << found << endl;

  if(!found)
    return region.end();
  return field_it;
}

void analysis::summary_memory_state::topify(region_t &region, int64_t offset,
    size_t size) {
  auto field_it = retrieve_kill(region, offset, size);
  if(field_it != region.end()) {
    num_var nv(field_it->second.num_id);
    child_state->kill({&nv});
    /*
     * Todo: Erasing variables is tricky now...
     */
//    region.erase(field_it->first);
  }
}

id_shared_t analysis::summary_memory_state::transVarReg(io_region io, int64_t offset, size_t size) {
  auto field_in_it = retrieve_kill(io.in_r, offset, size);
  auto field_out_it = retrieve_kill(io.out_r, offset, size);
  id_shared_t r;
  if(field_in_it == io.in_r.end()) {
    assert(field_out_it == io.out_r.end());
    field &f = io.insert(child_state, offset, size);
    r = f.num_id;
  } else {
    assert(field_out_it != io.out_r.end());
    r = field_out_it->second.num_id;
  }
  return r;
}

id_shared_t analysis::summary_memory_state::transVar(id_shared_t var_id, int64_t offset, size_t size) {
  return transVarReg(region_by_id(&relation::get_regions, var_id), offset, size);
}

vector<field> analysis::summary_memory_state::transLERegFields(region_t &region, int64_t offset, size_t size) {
  vector<field> fields;
  int64_t consumed = 0;
  while(true) {
    auto field_it = region.find(offset + consumed);
    if(field_it == region.end()) {
      fields.clear();
      break;
    } else {
      field &f = field_it->second;
      if(f.size == 0) {
        fields.clear();
        break;
      }
      size_t size_rest = size - consumed;
      if(size_rest < f.size) {
        fields.clear();
        break;
      } else {
        fields.push_back(f);
        if(size_rest == f.size)
          break;
      }
      consumed += f.size;
    }
  }
  return fields;
}

num_linear *analysis::summary_memory_state::assemble_fields(vector<field> fields) {
  assert(fields.size() > 0);
  if(fields.size() == 1) {
    return new num_linear_term(new num_var(fields[0].num_id));
  } else {
    num_linear *l = new num_linear_term(new num_var(fields[0].num_id));
    size_t size_acc = fields[0].size;
    for(size_t i = 1; i < fields.size(); i++) {
      l = new num_linear_term((size_t)1 << size_acc, new num_var(fields[i].num_id), l);
      size_acc += fields[i].size;
    }
    return l;
  }
}

num_linear *analysis::summary_memory_state::transLEReg(io_region io, int64_t offset, size_t size) {
//  cout << "transLEReg "<< offset << ":" << size << endl;
//  if(offset == 8 && size == 8)
//    printf("Juhu\n");

  vector<field> fields = transLERegFields(io.out_r, offset, size);

//  cout << "Number of fields: " << fields.size() << endl;

  if(fields.size() == 0) {
    if(overlap_region(io.out_r, offset, size))
      return new num_linear_vs(value_set::top);
    else {
      field &f = io.insert(child_state, offset, size);
      return new num_linear_term(new num_var(f.num_id));
    }
  } else return assemble_fields(fields);
}

num_linear *analysis::summary_memory_state::transLE(id_shared_t var_id, int64_t offset, size_t size) {
  io_region io = region_by_id(&relation::get_regions, var_id);
  return transLEReg(io, offset, size);
}

num_linear *analysis::summary_memory_state::transLEInput(id_shared_t var_id, int64_t offset, size_t size) {

  auto id_in_it = input.regions.find(var_id);
  if(id_in_it == input.regions.end())
    return new num_linear_vs(value_set::top);
  region_t &region = id_in_it->second;

  vector<field> fields = transLERegFields(region, offset, size);

  if(fields.size() == 0) {
    return new num_linear_vs(value_set::top);
  } else return assemble_fields(fields);
}

analysis::summary_memory_state::summary_memory_state(shared_ptr<static_memory> sm, numeric_state *child_state, bool start_bottom) :
    sm(sm), child_state(child_state) {
  if(start_bottom) {
    /*
     * start value
     */
  } else {
    /*
     * bottom
     */
  }
}

bool analysis::summary_memory_state::is_bottom() {
  return child_state->is_bottom();
}

bool analysis::summary_memory_state::operator >=(const domain_state &other) const {
  summary_memory_state const &other_casted = dynamic_cast<summary_memory_state const&>(other);
  numeric_state *me_compat;
  numeric_state *other_compat;
  tie(ignore, me_compat, other_compat) = compat(this, &other_casted);
  bool result = *me_compat >= *other_compat;
  delete me_compat;
  delete other_compat;
  return result;
}

summary_memory_state *analysis::summary_memory_state::join(domain_state *other, size_t current_node) {
  return domop(other, current_node, &numeric_state::join);
}

summary_memory_state *analysis::summary_memory_state::widen(domain_state *other, size_t current_node) {
  return domop(other, current_node, &numeric_state::widen);
}

summary_memory_state *analysis::summary_memory_state::narrow(domain_state *other, size_t current_node) {
  return domop(other, current_node, &numeric_state::narrow);
}

summary_memory_state *analysis::summary_memory_state::apply_summary(summary_memory_state *summary) {
  numeric_state *child_met = child_state->meet(summary->child_state, 0);
  summary_memory_state *summary_applied = new summary_memory_state(sm, child_met, input, output);


  for(auto &region_mapping : summary->input.regions) {
    for(auto &field_mapping : region_mapping.second) {
      field &f = field_mapping.second;
      num_linear *l_s_in = summary->transLEInput(region_mapping.first, field_mapping.first, f.size);
      num_linear *l_call_out = summary_applied->transLE(region_mapping.first, field_mapping.first, f.size);
      num_expr_cmp *nec = num_expr_cmp::equals(l_call_out, l_s_in);
//      cout << "nec: " << *nec << endl;
      summary_applied->child_state->assume(nec);
      delete nec;
      delete l_call_out;
      delete l_s_in;
    }
  }

//  cout << "summary_applied:" << endl;
//  cout << *summary_applied << endl;

  for(auto &region_mapping : summary->output.regions) {
    for(auto &field_mapping : region_mapping.second) {
      field &f = field_mapping.second;
      num_linear *l_out = summary->transLE(region_mapping.first, field_mapping.first, f.size);
      num_expr *l_out_expr = new num_expr_lin(l_out);
      num_var *v_update = new num_var(summary_applied->transVar(region_mapping.first, field_mapping.first, f.size));
      cout << *v_update << " <- " << *l_out_expr << endl;
      summary_applied->child_state->assign(v_update, l_out_expr);
      delete l_out_expr;
      delete v_update;
    }
  }

  num_vars *_vars = summary_applied->vars_relations();
  summary_applied->project(_vars);
  delete _vars;

  return summary_applied;
}

void analysis::summary_memory_state::update(gdsl::rreil::assign *assign) {
  if(is_bottom()) return;

  variable *var = assign->get_lhs();
  id_shared_t num_id = transVar(shared_copy(var->get_id()), var->get_offset(), assign->get_size());
  num_var *n_var = new num_var(num_id);
  converter cv(assign->get_size(), [&](shared_ptr<gdsl::rreil::id> id, size_t offset, size_t size) {
    return transLE(id, offset, size);
  });
  num_expr *n_expr = cv.conv_expr(assign->get_rhs());
  child_state->assign(n_var, n_expr);
  delete n_expr;
  delete n_var;

  cleanup();
}

summary_memory_state *analysis::summary_memory_state::copy() const {
  return new summary_memory_state(*this);
}

summary_memory_state *analysis::summary_memory_state::start_value(shared_ptr<static_memory> sm, numeric_state *start_num) {
  return new summary_memory_state(sm, start_num, true);
}

summary_memory_state *analysis::summary_memory_state::bottom(shared_ptr<static_memory> sm, numeric_state *bottom_num) {
  return new summary_memory_state(sm, bottom_num, false);
}

void analysis::summary_memory_state::update(gdsl::rreil::load *load) {
  if(is_bottom()) return;

  address *addr = load->get_address();
  auto temp = assign_temporary(addr->get_lin(), addr->get_size());
  vector<num_linear*> lins;
  ptr_set_t aliases = child_state->queryAls(temp->get_var());
  for(auto &alias : aliases) {
//    cout << "Alias: " << *alias.id << "@" << *alias.offset << endl;
    io_region io = dereference(alias.id);

    bool is_static = false;
    void *symbol_address;
    tie(is_static, symbol_address) = static_address(alias.id);

    value_set_visitor vsv;
    vsv._([&](vs_finite *v) {
      if(v->is_bottom()) {
        /*
         * Todo: handle bottom
         */
        return;
      }
      set<int64_t> overlapping;
      set<int64_t> non_overlapping;
      tie(overlapping, non_overlapping) = overlappings(v, load->get_size());

      if(overlapping.size() > 0)
        return;

      for(auto noo : non_overlapping) {
        if(is_static)
          initialize_static(io, symbol_address, noo, load->get_size());
        lins.push_back(transLEReg(io, noo, load->get_size()));
      }
    });
    vsv._([&](vs_open *o) {
    });
    vsv._([&](vs_top *t) {
    });
    vs_shared_t offset_bits = *vs_finite::single(8)*alias.offset;
    offset_bits->accept(vsv);
  }

  num_var *lhs = new num_var(
      transVar(shared_copy(load->get_lhs()->get_id()), load->get_lhs()->get_offset(), load->get_size()));
  if(lins.size() == 0) {
    num_expr *rhs_expr = new num_expr_lin(new num_linear_vs(value_set::top));
    child_state->assign(lhs, rhs_expr);
    delete rhs_expr;
  } else if(lins.size() == 1) {
    num_expr *rhs_expr = new num_expr_lin(lins[0]);
    child_state->assign(lhs, rhs_expr);
    delete rhs_expr;
  } else {
//      cout << "Weak assign after load" << endl;
    num_var *temp = new num_var(numeric_id::generate());
    num_expr *rhs_first = new num_expr_lin(lins[0]);
    child_state->assign(temp, rhs_first);
    delete rhs_first;
    for(size_t i = 1; i < lins.size(); i++) {
      num_expr *rhs_next = new num_expr_lin(lins[i]);
      child_state->weak_assign(temp, rhs_next);
      delete rhs_next;
    }
    num_expr *rhs_copy = new num_expr_lin(new num_linear_term(temp->copy()));
    child_state->assign(lhs, rhs_copy);
    delete rhs_copy;
    child_state->kill({temp});
    delete temp;
  }
  delete lhs;

  cleanup();
}

void analysis::summary_memory_state::update(gdsl::rreil::store *store) {
  if(is_bottom()) return;

  address *addr = store->get_address();
  auto temp = assign_temporary(addr->get_lin(), addr->get_size());

  ptr_set_t aliases = child_state->queryAls(temp->get_var());
  for(auto &alias : aliases) {
    io_region io = dereference(alias.id);

    bool is_static = false;
    tie(is_static, ignore) = static_address(alias.id);
    if(is_static) {
      cout << "Warning: Ignoring possible store to static memory" << endl;
      continue;
    }

    vector<id_shared_t> ids;

    bool singleton = aliases.size() == 1;
    bool _continue = false;
    value_set_visitor vsv;
    vsv._([&](vs_finite *v) {
      singleton = singleton && v->is_singleton();
      if(v->is_bottom()) {
        /*
         * Todo: handle bottom
         */
        _continue = true;
        return;
      }

      set<int64_t> overlapping;
      set<int64_t> non_overlapping;
      tie(overlapping, non_overlapping) = overlappings(v, store->get_size());

      for(auto oo : overlapping)
        topify(io.out_r, oo, store->get_size());
      for(auto noo : non_overlapping)
        ids.push_back(transVarReg(io, noo, store->get_size()));
    });
    vsv._([&](vs_open *o) {
      singleton = false;
      switch(o->get_open_dir()) {
        case UPWARD: {
          for(auto field_it = io.out_r.begin(); field_it != io.out_r.end(); field_it++)
            if(field_it->first + field_it->second.size > o->get_limit())
              topify(io.out_r, field_it->first, store->get_size());
          break;
        }
        case DOWNWARD: {
          for(auto field_it = io.out_r.begin(); field_it != io.out_r.end(); field_it++) {
            if(field_it->first < o->get_limit())
              topify(io.out_r, field_it->first, store->get_size());
            else if(field_it->first < o->get_limit() + store->get_size())
              topify(io.out_r, field_it->first, store->get_size());
          }
          break;
        }
      }

    });
    vsv._([&](vs_top *t) {
      _continue = true;
      singleton = false;
    });
    vs_shared_t offset_bits = *vs_finite::single(8)*alias.offset;
    offset_bits->accept(vsv);
    /*
     * Erasing from regions is tricky now...
     */
//    if(region.size() == 0) {
//      deref.erase(alias.id);
//      continue;
//    }
    if(_continue)
      continue;

    converter rhs_cv(store->get_size(), [&](shared_ptr<gdsl::rreil::id> id, size_t offset, size_t size) {
      return transLE(id, offset, size);
    });
    for(auto id : ids) {
      num_var *lhs = new num_var(id);
      num_expr *rhs = rhs_cv.conv_expr(store->get_rhs());
      if(singleton)
        child_state->assign(lhs, rhs);
      else
        child_state->weak_assign(lhs, rhs);
      delete lhs;
      delete rhs;
    }
  }

  cleanup();
}

void analysis::summary_memory_state::assume(gdsl::rreil::sexpr *cond) {
  converter cv(0, [&](shared_ptr<gdsl::rreil::id> id, size_t offset, size_t size) {
    return transLE(id, offset, size);
  });
  expr_cmp_result_t ecr = cv.conv_expr_cmp(cond);
  child_state->assume(ecr.primary);
  for(auto add : ecr.additional)
    child_state->assume(add);
  ecr.free();
  unique_ptr<managed_temporary> temp = assign_temporary(cond, 1);
  vs_shared_t value = child_state->queryVal(temp->get_var());
  if(*value == vs_finite::_false)
    bottomify();
}

void analysis::summary_memory_state::assume_not(gdsl::rreil::sexpr *cond) {
  converter cv(0, [&](shared_ptr<gdsl::rreil::id> id, size_t offset, size_t size) {
    return transLE(id, offset, size);
  });
  expr_cmp_result_t ecr = cv.conv_expr_cmp(cond);
  num_expr_cmp *ec_primary_not = ecr.primary->negate();
  child_state->assume(ec_primary_not);
  delete ec_primary_not;
  for(auto add : ecr.additional)
    /*
     * Additional constraints must not be negated!
     */
    child_state->assume(add);
  ecr.free();
  unique_ptr<managed_temporary> temp = assign_temporary(cond, 1);
  vs_shared_t value = child_state->queryVal(temp->get_var());
  if(*value == vs_finite::_true)
    bottomify();
}

void analysis::summary_memory_state::cleanup() {
  auto _inner = [&](auto &regions) {
    auto region_it = regions.begin();
    while(region_it != regions.end()) {
      auto field_it = region_it->second.begin();
      while(field_it != region_it->second.end()) {
        num_var *nv = new num_var(field_it->second.num_id);
        child_state->cleanup(nv);
//        if(!child_state->cleanup(nv))
//        region_it->second.erase(field_it++);
//        else
        field_it++;
        delete nv;
      }
//      if(region_it->second.size() == 0)
//      regions.erase(region_it++);
//      else
      region_it++;
    }
  };
  _inner(input.regions);
  _inner(input.deref);
  _inner(output.regions);
  _inner(output.deref);
}

void analysis::summary_memory_state::project(api::num_vars *vars) {
  /*
   * Todo: Check for additional vars in memory regions
   */
  child_state->project(vars);
}

api::num_vars *analysis::summary_memory_state::vars_relations() {
  id_set_t known_ids;

  auto _inner = [&](auto &regions) {
    auto region_it = regions.begin();
    while(region_it != regions.end()) {
      auto field_it = region_it->second.begin();
      while(field_it != region_it->second.end()) {
        known_ids.insert(field_it->second.num_id);
        cout << "adding " << *field_it->second.num_id << endl;
        field_it++;
      }
      region_it++;
    }
  };
  _inner(input.regions);
  _inner(input.deref);
  _inner(output.regions);
  _inner(output.deref);

  return new num_vars(known_ids);
}

api::num_vars *analysis::summary_memory_state::vars() {
  num_vars *_vars = child_state->vars();

  num_vars *relations_vars = vars_relations();
  _vars->add(relations_vars->get_ids());

  return _vars;
}

std::unique_ptr<managed_temporary> analysis::summary_memory_state::assign_temporary(gdsl::rreil::expr *e, int_t size) {
  return assign_temporary(size, [&](converter &cv) {
    return cv.conv_expr(e);
  });
}

std::unique_ptr<managed_temporary> analysis::summary_memory_state::assign_temporary(gdsl::rreil::linear *l, int_t size) {
  return assign_temporary(size, [&](converter &cv) {
    return cv.conv_expr(l);
  });
}

std::unique_ptr<managed_temporary> analysis::summary_memory_state::assign_temporary(gdsl::rreil::sexpr *se, int_t size) {
  return assign_temporary(size, [&](converter &cv) {
    return cv.conv_expr(se);
  });
}

summy::vs_shared_t analysis::summary_memory_state::queryVal(gdsl::rreil::linear *l, size_t size) {
  converter cv(size, [&](shared_ptr<gdsl::rreil::id> id, size_t offset, size_t size) {
    return transLE(id, offset, size);
  });
  num_linear *nl = cv.conv_linear(l);
  summy::vs_shared_t result = child_state->queryVal(nl);
  delete nl;
  return result;
}

summy::vs_shared_t analysis::summary_memory_state::queryVal(gdsl::rreil::expr *e, size_t size) {
  unique_ptr<managed_temporary> temp = assign_temporary(e, size);
  summy::vs_shared_t result = child_state->queryVal(temp->get_var());
  return result;
}

std::set<summy::vs_shared_t> analysis::summary_memory_state::queryPts(std::unique_ptr<managed_temporary> &address) {
  throw string("analysis::summary_memory_state::queryPts(std::unique_ptr<managed_temporary>&)");
//  std::set<summy::vs_shared_t> result;
//  ptr_set_t aliases = child_state->queryAls(address->get_var());
//  for(auto &alias : aliases) {
//    io_region io = dereference(alias.id);
//    auto zero_it = io.out_r.find(0);
//    if(io.out_r.find(0) == io.out_r.end()) {
////      tie(zero_it, ignore) = region.insert(make_pair(0, field { 64,
//    }
////        numeric_id::generate() }));
//    num_linear *lin = transLE(zero_it->second.num_id, 0, 64);
//    vs_shared_t child_val = child_state->queryVal(lin);
//    result.insert(child_val);
//    delete lin;
//  }
//  return result;
}

api::ptr_set_t analysis::summary_memory_state::queryAls(gdsl::rreil::address *a) {
  if(is_bottom()) return ptr_set_t {};
  auto temp = assign_temporary(a->get_lin(), a->get_size());
  ptr_set_t aliases = child_state->queryAls(temp->get_var());
  return aliases;
}

//const region_t &analysis::summary_memory_state::query_region(id_shared_t id) {
//  return regions[id];
//}

std::tuple<summary_memory_state::memory_head, numeric_state*, numeric_state*> analysis::summary_memory_state::compat(
    const summary_memory_state *a, const summary_memory_state *b) {
  numeric_state *a_n = a->child_state->copy();
  numeric_state *b_n = b->child_state->copy();

//  if(!a_n->is_bottom() && !b_n->is_bottom()) {
//    cout << "++++++++++++++++++++++++++++++" << endl;
//    cout << "++++++++++++++++++++++++++++++" << endl;
//    cout << "++++++++++++++++++++++++++++++" << endl;
//    cout << "compat OF" << endl;
//    cout << *a << endl;
//    cout << "WITH" << endl;
//    cout << *b << endl;
//  }

  auto join_region_map = [&](region_map_t const &a_map, region_map_t const &b_map) {
    region_map_t result_map;

    auto handle_region = [&](id_shared_t id, region_t const &region_a, region_t const &region_b) {
      num_var_pairs_t equate_kill_vars;
      for(auto &field_it : region_a) {
        auto field_b_it = region_b.find(field_it.first);
        if(field_b_it != region_b.end()) {
          field const &f = field_it.second;
          field const &f_b = field_b_it->second;
          if(!(*f.num_id == *f_b.num_id)) {
            equate_kill_vars.push_back(make_tuple(new num_var(f.num_id), new num_var(f_b.num_id)));
//            cout << "Some vars get equate-killed..." << *f.num_id << " / " << *f_b.num_id << endl;
//            cout << "handle id..." << *id << endl;
          }
        }
      }
      b_n->equate_kill(equate_kill_vars);
      for(auto pair : equate_kill_vars) {
        num_var *a, *b;
        tie(a, b) = pair;
        delete a;
        delete b;
      }

      //      region_map_t::iterator head_region_it;
      //      tie(head_region_it, ignore) = result_map.insert(make_pair(id, region_t()));
      //      region_t &region = head_region_it->second;
      region_t region;

      auto join = [&](numeric_state *n, region_t const &from, region_t const &to) {
        auto kill = [&](id_shared_t id) {
          num_var *nv = new num_var(id);
          n->kill( {nv});
          delete nv;
        };

      //        cout << "n equals " << (a_n == n ? "a" : "b") << endl;

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

      join(b_n, region_a, region_b);
      join(a_n, region_b, region_a);

      if(region.size() > 0)
        result_map.insert(make_pair(id, region));
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
  head.input.regions = join_region_map(a->input.regions, b->input.regions);
  head.input.deref = join_region_map(a->input.deref, b->input.deref);
  head.output.regions = join_region_map(a->output.regions, b->output.regions);
  head.output.deref = join_region_map(a->output.deref, b->output.deref);

//  if(!a_n->is_bottom() && !b_n->is_bottom()) {
//    cout << "Result #1" << endl;
//    cout << summary_memory_state(a_n->copy(), head.regions, head.deref) << endl << endl;
//    cout << "Result #2" << endl;
//    cout << summary_memory_state(b_n->copy(), head.regions, head.deref) << endl << endl;
//
//    cout << "++++++++++++++++++++++++++++++" << endl;
//    cout << "++++++++++++++++++++++++++++++" << endl;
//    cout << "++++++++++++++++++++++++++++++" << endl;
//  }
  return make_tuple(head, a_n, b_n);
}
