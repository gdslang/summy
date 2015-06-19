/*
 * summary_memory_state.cpp
 *
 *  Created on: Mar 13, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/api/api.h>
#include <summy/analysis/domains/api/numeric/converter.h>
#include <summy/analysis/domains/summary_memory_state.h>
#include <summy/analysis/domains/merge_region_iterator.h>
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
#include <queue>
#include <experimental/optional>

using namespace analysis::api;
using namespace summy;
using namespace gdsl::rreil;
using namespace summy::rreil;


using namespace analysis;
using namespace std;
using namespace std::experimental;

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

//  cout << "assume " << *n_in << " aliases " << ptr(shared_ptr<gdsl::rreil::id>(new memory_id(0, nid_in)), vs_finite::zero) << endl;

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

io_region analysis::summary_memory_state::region_by_id(regions_getter_t getter, id_shared_t id) {
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

//region_t analysis::summary_memory_state::merge_memory(id_shared_t addr_a, region_t &r_b) {
//  io_region io_a = dereference(addr_a);
//
//  region_t merged;
//
//  auto a_it = io_a.out_r.begin();
//  auto b_it = r_b.begin();
//  while(a_it != io_a.out_r.end() && b_it != r_b.end()) {
//    int64_t offset_a = a_it->first;
//    int64_t offset_b = b_it->first;
//    if(offset_a < offset_b)
//      a_it++;
//    else if(offset_b < offset_a)
//      b_it++;
//    else {
//      field &f_a = a_it->second;
//      field &f_b = b_it->second;
//      if(f_a.size == f_b.size) {
//        id_shared_t n_id = numeric_id::generate();
//
//        num_var *n_var = new num_var(n_id);
//        num_expr *expr_a = new num_expr_lin(new num_linear_term(new num_var(f_a.num_id)));
//        num_expr *expr_b = new num_expr_lin(new num_linear_term(new num_var(f_b.num_id)));
//        child_state->assign(n_var, expr_a);
//        child_state->weak_assign(n_var, expr_b);
//        delete expr_b;
//        delete expr_a;
//        delete n_var;
//
//        merged.insert(make_pair(offset_a, field { f_a.size, n_id } ));
//      }
//      a_it++;
//      b_it++;
//    }
//  }
//
////  id_shared_t merged_key = make_shared<memory_id>(0, numeric_id::generate());
////  input.deref.insert(make_pair(merged_key, merged));
////  /*
////   * Todo: Same numeric variables in input and output?
////   */
////  output.deref.insert(make_pair(merged_key, merged));
//
//  return merged;
//}
//
//region_t analysis::summary_memory_state::merge_memory(id_shared_t addr_a, id_shared_t addr_b) {
//  io_region io_b = dereference(addr_b);
//
//  return merge_memory(addr_a, io_b.out_r);
//}


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
        size_t first_size = offset - offset_next;
        replacements.push_back(make_tuple(offset_next, first_size));
        if(f_next.size > size + first_size)
          replacements.push_back(make_tuple(offset + size, f_next.size - size - first_size));
      } else if(f_next.size > size)
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

id_shared_t analysis::summary_memory_state::transDeref(id_shared_t var_id, int64_t offset, size_t size) {
  return transVarReg(region_by_id(&relation::get_deref, var_id), offset, size);
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

//  for(auto &f : fields)
//    cout << "Field: " << *f.num_id << endl;

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

num_linear *analysis::summary_memory_state::transLE(regions_getter_t rget, id_shared_t var_id, int64_t offset, size_t size) {
  io_region io = region_by_id(rget, var_id);
  return transLEReg(io, offset, size);
}

num_linear *analysis::summary_memory_state::transLE(id_shared_t var_id, int64_t offset, size_t size) {
//  cout << "transLE(" << *var_id << ", ...)" << endl;
  return transLE(&relation::get_regions, var_id, offset, size);
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

analysis::summary_memory_state::summary_memory_state(shared_ptr<static_memory> sm, numeric_state *child_state,
    bool start_bottom) :
    memory_state_base(child_state), sm(sm) {
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

bool analysis::summary_memory_state::is_bottom() const {
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
  summary_memory_state *return_site = copy();
  /*
   * Each of the variables of the summary maps to a number of variables in the caller at
   * certain offsets. The mapping is established from the structure of the memory
   * of the summary.
   */
  map<id_shared_t, ptr_set_t, id_less_no_version> ptr_mapping;

  typedef std::set<id_shared_t, id_less_no_version> alias_queue_t;
  alias_queue_t ptr_worklist;

  auto build_pmap_region = [&](id_shared_t region_key_summary, ptr_set_t const &region_keys_c, regions_getter_t rgetter) {
    auto next_ids = (summary->input.*rgetter)().find(region_key_summary);
    if(next_ids == (summary->input.*rgetter)().end())
      return;

    region_t &region_s = next_ids->second;

    for(auto &field_mapping_s : region_s) {
      field &f_s = field_mapping_s.second;
      num_var *nv_field_s = new num_var(f_s.num_id);
      ptr_set_t aliases_fld_s = summary->child_state->queryAls(nv_field_s);

      assert(aliases_fld_s.size() <= 1);

      /*
       * Todo: Warning if an alias is found in the summary plus this alias has a region in the deref map
       * and no alias is found in 'c'
       * Todo: What about an alias in 'me' with no alias in the summary? We should somehow remove the alias in 'c'
       * then
       */

      ptr_set_t aliases_fld_c;

      updater_t record_aliases = [&](num_var *nv_flc_c) {
//          cout << "New mapping in region " << *next_me << " from " << *f_s.num_id << " to " << *id_me << endl;
//        variable_mapping[f_s.num_id].insert(ptr(nv_me->get_id(), vs_finite::zero));
        ptr_set_t aliases_fld_c_next = return_site->child_state->queryAls(nv_flc_c);
        /*
         * Record new aliases...
         */
        aliases_fld_c.insert(aliases_fld_c_next.begin(), aliases_fld_c_next.end());
      };

      return_site->update_multiple(region_keys_c, rgetter, f_s.size, record_aliases, record_aliases);
      delete nv_field_s;

      for(auto &p_s : aliases_fld_s) {
        ptr_set_t &aliases_c = ptr_mapping[p_s.id];
        if(!includes(aliases_c.begin(), aliases_c.end(), aliases_fld_c.begin(), aliases_fld_c.end())) {
          aliases_c.insert(aliases_fld_c.begin(), aliases_fld_c.end());
          ptr_worklist.insert(p_s.id);
        }
      }
    }
  };

  /*
   * Build the variable mapping from the input. Here, we consider the regions
   * from the 'regions' part of the memory only.
   */

  for(auto &region_mapping_si : summary->input.regions) {
    id_shared_t region_key_summary = region_mapping_si.first;
    ptr_set_t region_keys_c = ptr_set_t({ptr(region_key_summary, vs_finite::zero)});

    build_pmap_region(region_key_summary, region_keys_c, &relation::get_regions);
  }

  /*
   * Memory matching
   *
   * => We use the input for field names and aliases
   * => We use the output for field names only
   */

  do {
    alias_queue_t alias_queue = ptr_worklist;
    ptr_worklist.clear();
    while(!alias_queue.empty()) {
      auto aq_first_it = alias_queue.begin();
      id_shared_t region_key_summary = *aq_first_it;
      alias_queue.erase(aq_first_it);

//      cout << "next_s: " << *next_s << endl;

      /*
       * First, we match field names and alias from the
       * memory (deref) input
       */
      ptr_set_t const &region_keys_c = ptr_mapping.at(region_key_summary);

      build_pmap_region(region_key_summary, region_keys_c, &relation::get_deref);

    }

  } while(!ptr_worklist.empty());

  /*
   * Having built the relation between variable and pointers names of the caller state and the summary,
   * we apply the summary by updating the caller using the summary state.
   */

  /*
   * Application in regions, one alias
   */

  num_expr *_top = new num_expr_lin(new num_linear_vs(value_set::top));
  auto process_region = [&](regions_getter_t getter, ptr_set_t &region_aliases_c, region_t &region) {
    for(auto &field_mapping_s : region) {
      field &f_s = field_mapping_s.second;
//      id_shared_t id_me = me_copy->transVar(region_key, field_mapping_s.first, f_s.size);

//      cout << "    " << *id_me << endl;

      num_var *nv_s = new num_var(f_s.num_id);
      ptr_set_t aliases_s = summary->child_state->queryAls(nv_s);
      delete nv_s;

      ptr_set_t aliases_c;
      for(auto &alias_s : aliases_s) {
        auto aliases_mapped_it = ptr_mapping.find(alias_s.id);
        ptr_set_t const &aliases_c_next = aliases_mapped_it->second;
//        assert(aliases_mapped_it != alias_map.end() && aliases_me_ptr.size() > 0);
//        cout << "search result for " << *_ptr.id << ": " << (aliases_mapped_it != alias_map.end()) << endl;
        if(aliases_mapped_it != ptr_mapping.end())
          for(auto alias_c_next : aliases_c_next)
            aliases_c.insert(ptr(alias_c_next.id, *alias_c_next.offset + alias_s.offset));
      }

      updater_t strong = [&](api::num_var *nv_fld_c) {
//        cout << "strong for " << *nv_me << ": " << aliases_me << endl;
        return_site->child_state->assign(nv_fld_c, _top);
        return_site->child_state->assume(nv_fld_c, aliases_c);
      };
      updater_t weak = [&](api::num_var *nv_fld_c) {
//        cout << "weak for " << *nv_me << ": " << aliases_me << endl;
        ptr_set_t aliases_joined_c = return_site->child_state->queryAls(nv_fld_c);
        return_site->child_state->assign(nv_fld_c, _top);
        aliases_joined_c.insert(aliases_c.begin(), aliases_c.end());
        return_site->child_state->assume(nv_fld_c, aliases_joined_c);
      };
      return_site->update_multiple(region_aliases_c, getter, f_s.size, strong, weak);
    }
  };

  for(auto &region_mapping_so : summary->output.regions) {
    id_shared_t region_key = region_mapping_so.first;
    ptr_set_t region_aliases_c = ptr_set_t({ ptr(region_key, vs_finite::zero) });
    process_region(&relation::get_regions, region_aliases_c, region_mapping_so.second);
  }

  for(auto &deref_mapping_so : summary->output.deref) {
    id_shared_t region_key = deref_mapping_so.first;
    ptr_set_t &region_aliases_c = ptr_mapping.at(region_key);
    process_region(&relation::get_deref, region_aliases_c, deref_mapping_so.second);
  }
  delete _top;

  num_vars *_vars = return_site->vars_relations();
  return_site->project(_vars);
  delete _vars;

  return return_site;
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
//    cout << "Load Alias: " << *alias.id << "@" << *alias.offset << endl;
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

void analysis::summary_memory_state::update_multiple(api::ptr_set_t aliases, regions_getter_t getter, size_t size, updater_t strong, updater_t weak) {
  for(auto &alias : aliases) {

    io_region io = region_by_id(getter, alias.id);

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
      tie(overlapping, non_overlapping) = overlappings(v, size);

      for(auto oo : overlapping)
        topify(io.out_r, oo, size);
      for(auto noo : non_overlapping)
        ids.push_back(transVarReg(io, noo, size));
    });
    vsv._([&](vs_open *o) {
      singleton = false;
      switch(o->get_open_dir()) {
        case UPWARD: {
          for(auto field_it = io.out_r.begin(); field_it != io.out_r.end(); field_it++)
            if(field_it->first + field_it->second.size > o->get_limit())
              topify(io.out_r, field_it->first, size);
          break;
        }
        case DOWNWARD: {
          for(auto field_it = io.out_r.begin(); field_it != io.out_r.end(); field_it++) {
            if(field_it->first < o->get_limit())
              topify(io.out_r, field_it->first, size);
            else if(field_it->first < o->get_limit() + size)
              topify(io.out_r, field_it->first, size);
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

    for(auto id : ids) {
      num_var *lhs = new num_var(id);
      if(singleton)
        strong(lhs);
      else
        weak(lhs);
      delete lhs;
    }
  }
}

void analysis::summary_memory_state::store(api::ptr_set_t aliases, size_t size, api::num_expr *rhs) {
  update_multiple(aliases, &relation::get_deref, size, [&](num_var *lhs) {
    child_state->assign(lhs, rhs);
  }, [&](num_var *lhs) {
    child_state->weak_assign(lhs, rhs);
  });
}

void analysis::summary_memory_state::update(gdsl::rreil::store *store) {
  if(is_bottom()) return;

  address *addr = store->get_address();
  auto temp = assign_temporary(addr->get_lin(), addr->get_size());
  ptr_set_t aliases = child_state->queryAls(temp->get_var());

  converter rhs_cv(store->get_size(), [&](shared_ptr<gdsl::rreil::id> id, size_t offset, size_t size) {
    return transLE(id, offset, size);
  });
  num_expr *rhs = rhs_cv.conv_expr(store->get_rhs());
  this->store(aliases, store->get_size(), rhs);
  delete rhs;

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
   * ...?
   */
  auto project_regions = [&](region_map_t &regions) {
    for(auto regions_it = regions.begin(); regions_it != regions.end();) {
      if(vars->get_ids().find(regions_it->first) == vars->get_ids().end())
        regions.erase(regions_it++);
      else
        regions_it++;
    }
  };
  project_regions(input.deref);
  project_regions(output.deref);
  child_state->project(vars);
}

api::num_vars *analysis::summary_memory_state::vars_relations() {
  id_set_t known_ids;
  id_set_t worklist;

  auto for_region = [&](region_t &region) {
    auto field_it = region.begin();
    while(field_it != region.end()) {
      id_shared_t field_id = field_it->second.num_id;
      known_ids.insert(field_id);

      num_var *nv_id = new num_var(field_id);
      ptr_set_t aliases = child_state->queryAls(nv_id);
      delete nv_id;

      for(ptr const &p : aliases)
        worklist.insert(p.id);

      field_it++;
    }
  };

  auto for_regions = [&](region_map_t &regions) {
    auto region_it = regions.begin();
    while(region_it != regions.end()) {
      known_ids.insert(region_it->first);
      for_region(region_it->second);
      region_it++;
    }
  };

  for_regions(input.regions);
  for_regions(output.regions);

  while(!worklist.empty()) {
    id_shared_t next = *worklist.begin();
    worklist.erase(next);
    if(known_ids.find(next) != known_ids.end())
      continue;

    auto next_region = [&](region_map_t &regions) {
      auto regions_it = regions.find(next);
      if(regions_it != regions.end()) {
        for_region(regions_it->second);
      }
    };
    next_region(input.deref);
    next_region(output.deref);

    known_ids.insert(next);
  }

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

api::num_linear *analysis::summary_memory_state::dereference(api::num_var *v, int64_t offset, size_t size) {
  return transLE(&relation::get_deref, v->get_id(), offset, size);
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

api::ptr_set_t analysis::summary_memory_state::queryAls(api::num_var *v) {
  ptr_set_t aliases = child_state->queryAls(v);
  return aliases;
}

const region_t &analysis::summary_memory_state::query_region_output(id_shared_t id) {
  return output.regions[id];
}

num_var_pairs_t analysis::summary_memory_state::matchPointers(relation &a_in, relation &a_out, numeric_state *a_n,
    relation &b_in, relation &b_out, numeric_state *b_n) {
  /*
   * Find aliases for a specific region of the given summaries. We need both
   * input and output here in order to be able to add pointers that are missing
   * in one of the regions.
   */
  auto matchPointersRegion = [&](io_region &io_ra, io_region &io_rb) {
    num_var_pairs_t upcoming;

    vector<function<void()>> insertions;

    /*
     * We first check for missing pointers in ther region...
     */
    merge_region_iterator mri(io_ra.in_r, io_rb.in_r);
    while(mri != merge_region_iterator::end(io_ra.in_r, io_rb.in_r)) {
      region_pair_desc_t rpd = *mri;
      if(!rpd.collision) {
        if(!rpd.ending_last) {
//          cout << "fr: " << *rpd.ending_first.f.num_id << " at " << rpd.ending_first.offset << endl;

          field_desc_t ending_first = rpd.ending_first;
          if(ending_first.region_first)
            insertions.push_back([&io_rb, &b_n, ending_first]() {
//            cout << "Insertion into io_rb at " << ending_first.offset << endl;
              io_rb.insert(b_n, ending_first.offset, ending_first.f.size);
            });
          else
            insertions.push_back([&io_ra, &a_n, ending_first]() {
              io_ra.insert(a_n, ending_first.offset, ending_first.f.size);
            });
        }
      }
      ++mri;
    }

    /*
     * ... insert them...
     */
    for(auto inserter : insertions)
      inserter();

    /*
     * and finally retrieve all matching pointer variables. Keep in mind
     * that there is always at most one alias per numeric variable in
     * the input.
     */
    mri = merge_region_iterator(io_ra.in_r, io_rb.in_r);
    while(mri != merge_region_iterator::end(io_ra.in_r, io_rb.in_r)) {
      region_pair_desc_t rpd = *mri;
      if(!rpd.collision) {
        if(rpd.ending_last) {
          field const &f_a = rpd.ending_first.f;
          field const &f_b = rpd.ending_last.value().f;
          if(f_a.size == f_b.size) {
            num_var *f_a_nv = new num_var(f_a.num_id);
            ptr_set_t als_a = a_n->queryAls(f_a_nv);
            assert(als_a.size() <= 1);
            delete f_a_nv;

            num_var *f_b_nv = new num_var(f_b.num_id);
            ptr_set_t als_b = b_n->queryAls(f_b_nv);
            assert(als_b.size() <= 1);
            delete f_b_nv;

            if(als_a.size() == 1 && als_b.size() == 1) {
//              cout << "pushing aliases... " << endl;

              ptr p_a = *als_a.begin();
              assert(*p_a.offset == vs_finite::zero);
              ptr p_b = *als_b.begin();
              assert(*p_b.offset == vs_finite::zero);

              upcoming.push_back(make_tuple(new num_var(p_a.id), new num_var(p_b.id)));
            }
          }
        } else
          assert(false);
      }
      ++mri;
    }

    return upcoming;
  };

  struct region_pair {
    io_region io_ra;
    io_region io_rb;
  };
  queue<region_pair> worklist;

  /*
   * We first need to add all pointers referenced in the register part of the
   * state (that is, the regions map). Since the names of the region identifiers are global,
   * we only need to iterate both inputs.
   */
  auto init_from_regions = [&](region_map_t &first_in, region_map_t &first_out,
      region_map_t &second_in, region_map_t &second_out, bool a_b) {
    for(auto regions_first_it = first_in.begin(); regions_first_it != first_in.end(); regions_first_it++) {
      auto regions_second_it = second_in.find(regions_first_it->first);
      if(regions_second_it == second_in.end()) {
        tie(regions_second_it, ignore) = second_in.insert(make_pair(regions_first_it->first, region_t()));
        second_out.insert(make_pair(regions_first_it->first, region_t()));
      } else if(a_b)
        continue;

      auto &regions_first_out = first_out.at(regions_first_it->first);
      auto &regions_second_out = second_out.at(regions_first_it->first);
      io_region io_first = io_region { regions_first_it->second, regions_first_out };
      io_region io_second = io_region { regions_second_it->second, regions_second_out };
      if(a_b)
        worklist.push(region_pair { io_first, io_second });
      else
        worklist.push(region_pair { io_second, io_first });
    }
  };
  init_from_regions(a_in.regions, a_out.regions, b_in.regions, b_out.regions, true);
  init_from_regions(b_in.regions, b_out.regions, a_in.regions, a_out.regions, false);

  num_var_pairs_t result;

  /*
   * After collecting all matching pointers of the register, we need to match
   * the memory. Here, we need to recursively visit all referenced memory and
   * subsequently collect equalities in memory referenced by pointers that are
   * already known to be equal. Newly found memory regions are added to a work
   * list, we only finish when the worklist is empty.
   */
  while(!worklist.empty()) {
    region_pair rp = worklist.front();
    worklist.pop();

    /*
     * We first collect all equalities of the current region.
     */
    num_var_pairs_t upcoming = matchPointersRegion(rp.io_ra, rp.io_rb);
    for(auto upc : upcoming) {
      num_var *a;
      num_var *b;
      tie(a, b) = upc;
      /*
       * If the pointer variables have distinct names, we add them to the
       * result set of distinct pointers that need to be considered equal.
       */
      if(!(*a->get_id() == *b->get_id()))
        result.push_back(make_tuple(a->copy(), b->copy()));
    }

    /*
     * We add all corresponding memory regions as identified by the
     * respective pointers to the worklist. This way, their fields
     * will be matched at a later iteration.
     */
    for(auto vpair : upcoming) {
      num_var *va;
      num_var *vb;
      tie(va, vb) = vpair;

      auto deref_a_in_it = a_in.deref.find(va->get_id());
      auto deref_b_in_it = b_in.deref.find(vb->get_id());
      /*
       * If both pointers are not found in the deref map, they both have not
       * been deferenced. In this case, there is
       * nothing to match and we continue to the next item in the worklist.
       */
      if(deref_a_in_it == a_in.deref.end() && deref_b_in_it == b_in.deref.end()) {
        delete va;
        delete vb;
        continue;
      }
      /*
       * If only one pointer p_1 is found in its respective deref map, the other has
       * not been dereferenced. There may be fields in the dereferenced memory of
       * p_1 which need to be copied to other i/o relation. Therefore, we add the
       * missing memory region to the other relation.
       */
      else if(deref_a_in_it == a_in.deref.end()) {
        tie(deref_a_in_it, ignore) = a_in.deref.insert(make_pair(va->get_id(), region_t()));
        a_out.deref.insert(make_pair(va->get_id(), region_t()));
      } else if(deref_b_in_it == b_in.deref.end()) {
        tie(deref_b_in_it, ignore) = b_in.deref.insert(make_pair(vb->get_id(), region_t()));
        b_out.deref.insert(make_pair(vb->get_id(), region_t()));
      }
      auto &deref_a_out = a_out.deref.at(va->get_id());
      auto &deref_b_out = b_out.deref.at(vb->get_id());
      io_region io_a = io_region { deref_a_in_it->second, deref_a_out };
      io_region io_b = io_region { deref_b_in_it->second, deref_b_out };
      worklist.push(region_pair { io_a, io_b });

      delete va;
      delete vb;
    }
  }

  return result;
}

std::tuple<summary_memory_state::memory_head, numeric_state*, numeric_state*> analysis::summary_memory_state::compat(
    const summary_memory_state *a, const summary_memory_state *b) {
  numeric_state *a_n = a->child_state->copy();
  numeric_state *b_n = b->child_state->copy();

  if(a->is_bottom()) {
    memory_head head;
    head.input = b->input;
    head.output = b->output;
    return make_tuple(head, a_n, b_n);
  } else if(b->is_bottom()) {
    memory_head head;
    head.input = a->input;
    head.output = a->output;
    return make_tuple(head, a_n, b_n);
  }

//  if(!a_n->is_bottom() && !b_n->is_bottom()) {
//    cout << "++++++++++++++++++++++++++++++" << endl;
//    cout << "++++++++++++++++++++++++++++++" << endl;
//    cout << "++++++++++++++++++++++++++++++" << endl;
//    cout << "compat OF" << endl;
//    cout << *a << endl;
//    cout << "WITH" << endl;
//    cout << *b << endl;
//  }

  /*
   * Making two summary memory states compatible consists of two major steps: first, the structure of the
   * respective relations is matched. Second, one compatible relation is built from the modified relations
   * of the two states. Both steps also involve updating the numeric state, e.g. when introducing new
   * variables.
   */

  relation a_input = a->input;
  relation a_output = a->output;
  relation b_input = b->input;
  relation b_output = b->output;

  auto rename_rk = [&](relation &rel, id_shared_t from, id_shared_t to) {
//    cout << "rename_rk " << *from << " / " << *to << endl;

    auto rel_it = rel.deref.find(from);
    if(rel_it != rel.deref.end()) {
      region_t region = rel_it->second;
      rel.deref.erase(rel_it);
      rel.deref.insert(make_pair(to, region));
    }
  };

  /*
   * The first step is implemented by finding corresponding pointer variables...
   */
  num_var_pairs_t eq_aliases = matchPointers(a_input, a_output, a_n, b_input, b_output, b_n);

//  summary_memory_state *before_rename = new summary_memory_state(a->sm, b_n, b_input, b_output);
//  cout << "before_rename: " << *before_rename << endl;

  /*
   * ... and equating them in the respective numeric state. Additionally, the keys of the memory regions
   * in the deref map need to be replaced.
   */
  b_n->equate_kill(eq_aliases);
  for(auto &eq_alias : eq_aliases) {
    num_var *alias_a;
    num_var *alias_b;
    tie(alias_a, alias_b) = eq_alias;
    rename_rk(b_input, alias_b->get_id(), alias_a->get_id());
    rename_rk(b_output, alias_b->get_id(), alias_a->get_id());
    delete alias_a;
    delete alias_b;
  }

//  summary_memory_state *after_rename_b = new summary_memory_state(a->sm, b_n, b_input, b_output);
//  cout << "after_rename, b: " << *after_rename_b << endl;
//  summary_memory_state *after_rename_a = new summary_memory_state(a->sm, a_n, a_input, a_output);
//  cout << "after_rename, a: " << *after_rename_a << endl;

  /*
   * In the second step, all corresponding regions already have got the same region key. Thus,
   * in order to built a compatible relation we only need to iterate the regions in the deref and
   * regions map. For each pair of regions, the fields are matched. If a field perfectly overlaps
   * a field in the other region, the field is added to the compatible region. Otherwise, if there
   * is a conflict, a new field is created in the compatible region that contains a newly created
   * numeric variable and spans the whole range of the conflicting fields. It is not possible that
   * we find a field that neither conflicts nor perfectly overlaps since in that case the previous
   * step would have already added perfectly overlapping field in the other relation.
   */
  auto join_region_map = [&](region_map_t const &a_map, region_map_t const &b_map, bool input) {
    region_map_t result_map;

    auto handle_region = [&](id_shared_t id, region_t const &region_a, region_t const &region_b) {
      num_var_pairs_t equate_kill_vars;
      auto eqk_add = [&](id_shared_t x, id_shared_t y) {
        if(!(*x == *y))
          equate_kill_vars.push_back(make_tuple(new num_var(x), new num_var(y)));
      };
      id_set_t a_kill_ids;
      id_set_t b_kill_ids;

      region_t region;

      optional<int64_t> fn_from;
      optional<int64_t> fn_to;
      auto insert_f = [&]() {
        if(fn_from && fn_to) {
          field f = field  { (size_t)(fn_to.value() - fn_from.value()), numeric_id::generate() };
          region.insert(make_pair(fn_from.value(), f));
          fn_from = nullopt;
          fn_to = nullopt;
        }
      };
//      auto nsync = [&](id_shared_t id, numeric_state *from, numeric_state *to) {
//        assert(false);
////        if(!input)
////          return;
////        cout << "**id: " << *id << endl;
////        num_var *id_nv = new num_var(id);
////        ptr_set_t aliases = from->queryAls(id_nv);
////        for(auto a : aliases)
////          cout << "**alias: " << a << endl;
////        to->assume(id_nv, aliases);
////        delete id_nv;
//      };

      merge_region_iterator mri(region_a, region_b);
      while(mri != merge_region_iterator::end(region_a, region_b)) {
        region_pair_desc_t rpd = *mri;
        if(!rpd.ending_last)
        assert(false);
        if(rpd.collision) {
          field_desc_t fd_ending_first = rpd.ending_first;
          field_desc_t fd_ending_last = rpd.ending_last.value();

          fn_to = fd_ending_last.offset + fd_ending_last.f.size;
          if(!fn_from)
            fn_from = fd_ending_first.offset;
          a_kill_ids.insert(rpd.field_first_region().value().f.num_id);
          b_kill_ids.insert(rpd.field_second_region().value().f.num_id);
        } else {
          insert_f();

          field_desc_t fd_first = rpd.field_first_region().value();
          field_desc_t fd_second = rpd.field_second_region().value();

          eqk_add(fd_first.f.num_id, fd_second.f.num_id);
          region.insert(make_pair(fd_first.offset, fd_first.f));
        }
        ++mri;
      }

     b_n->equate_kill(equate_kill_vars);
     for(auto pair : equate_kill_vars) {
       num_var *a, *b;
       tie(a, b) = pair;
       delete a;
       delete b;
     }

     auto kill = [&](id_set_t kill_ids, numeric_state *ns) {
       for(auto id : kill_ids) {
         num_var *nv_id = new num_var(id);
         ns->kill({nv_id});
         delete nv_id;
       }
     };

     kill(a_kill_ids, a_n);
     kill(b_kill_ids, b_n);

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
        handle_region(region_b_it.first, region_t(), region_b_it.second);
    }
    return result_map;
  };

  memory_head head;
//  cout << "input_regions" << endl;
  head.input.regions = join_region_map(a_input.regions, b_input.regions, true);
//  cout << "input_deref" << endl;
  head.input.deref = join_region_map(a_input.deref, b_input.deref, true);
//  cout << "output_regions" << endl;
  head.output.regions = join_region_map(a_output.regions, b_output.regions, false);
//  cout << "output_deref" << endl;
  head.output.deref = join_region_map(a_output.deref, b_output.deref, false);

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
