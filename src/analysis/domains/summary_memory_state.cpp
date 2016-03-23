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
#include <cppgdsl/rreil/rreil.h>
#include <include/summy/rreil/id/id_visitor.h>
#include <algorithm>
#include <iosfwd>
#include <string>
#include <sstream>
#include <tuple>
#include <vector>
#include <assert.h>
#include <summy/analysis/domains/cr_merge_region_iterator.h>
#include <summy/analysis/domains/sms_op.h>
#include <summy/rreil/id/memory_id.h>
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

analysis::io_region::io_region(region_t &in_r, region_t &out_r, std::experimental::optional<id_shared_t const> r_key)
    : in_r(in_r), out_r(out_r) {
  if(r_key) name = r_key.value()->to_string();
}

optional<field> analysis::io_region::retrieve_field(numeric_state *child_state, int64_t offset, size_t size,
  bool replacement, bool handle_conflicts, std::function<ptr_set_t(id_shared_t)> ptr_set_ct) {
  //  cout << "INSERT offset=" << offset << ", size=" << size << ", replacement=" << replacement << endl;
  //  struct field_desc_t {
  //    int64_t offset;
  //    field f;
  //  };

  vector<int64_t> offsets;
  vector<field> replaced;
  bool contiguous = true;
  int64_t offset_next = offset;

  auto out_r_it = out_r.lower_bound(offset);

  /*
   * Check whether preceeding fields overlaps; in this case, we include the preceeding field
   */
  //  cout << "??? end? " << (out_r_it == out_r.end()) << endl;
  //  cout << "??? begin? " << (out_r_it == out_r.begin()) << endl;
  //  cout << "??? size? " << (out_r.size()) << endl;
  if(out_r_it != out_r.begin() && (out_r_it == out_r.end() || out_r_it->first > offset)) {
    --out_r_it;
    //    cout << ":-( " << out_r_it->first << endl;
    if(out_r_it->first + (int64_t)out_r_it->second.size <= offset) out_r_it++;
  }

  optional<int64_t> offset_first;
  optional<size_t> total_size;
  bool prefix_needed = false;
  bool suffix_needed = false;

  for(; out_r_it != out_r.end(); out_r_it++) {
    int64_t offset_current = out_r_it->first;
    if(offset_current >= offset + (int64_t)size) break;

//    cout << "In the way: offset:" << offset_current << " size: " << out_r_it->second.size << endl;

    if(offset_current < offset) prefix_needed = true;
    if(!offset_first) offset_first = offset_current;
    field &f = out_r_it->second;
    if(!total_size)
      total_size = f.size;
    else
      total_size = total_size.value() + f.size;

    offsets.push_back(offset_current);
    if(contiguous) {
      if(offset_current == offset_next) {
        field &f = out_r_it->second;
        replaced.push_back(f);
        offset_next += f.size;
      } else
        contiguous = false;
    }
  }
  if(!offset_first) {
    assert(!total_size);
    offset_first = offset;
    total_size = size;
  } else {
    assert(total_size);
    int64_t offset_after_existing = offset_first.value() + total_size.value();
    int64_t offset_after_new = offset + size;
    if(offset_after_existing > offset_after_new) {
      contiguous = false;
      suffix_needed = true;
    } else if(offset_after_new > offset_after_existing) {
      contiguous = false;
      total_size = total_size.value() + (offset_after_new - offset_after_existing);
    }
    if(offset < offset_first.value()) {
      total_size = total_size.value() + (offset_first.value() - offset);
      offset_first = offset;
    }
  }

  assert(offset_first);
  assert(total_size);

//  cout << "offset_first: " << offset_first.value() << endl;
//  cout << "total_size: " << total_size.value() << endl;

  //    contiguous = contiguous && (offset_next == offset + size);

  //  if(replaced.size() == 1) {
  //  cout << "offsets[0]: " << offsets[0] << endl;
  //  cout << "offset: " << offset << endl;
  //  }

  if(replaced.size() == 1 && offsets[0] == offset && replaced[0].size == size)
    return out_r.find(offset)->second;
  else if(!handle_conflicts && replaced.size() > 0)
    return nullopt;

  vector<num_var *> kill_vars;
  for(auto offset : offsets) {
    //    cout << "REMOVING AT OFFSET " << offset << endl;
    auto in_it = in_r.find(offset);
    auto out_it = out_r.find(offset);
    num_var *in_var = new num_var(in_it->second.num_id);
    num_var *out_var = new num_var(out_it->second.num_id);
    kill_vars.push_back(in_var);
    kill_vars.push_back(out_var);
    in_r.erase(in_it);
    out_r.erase(out_it);
  }

  ptr _nullptr = ptr(special_ptr::_nullptr, vs_finite::zero);
  ptr badptr = ptr(special_ptr::badptr, vs_finite::zero);

  auto insert_in = [&](int64_t offset, size_t size) {
    id_shared_t nid_in = name ? numeric_id::generate(name.value(), offset, size, true) : numeric_id::generate();
    num_var n_in(nid_in);
    field f_before = field{size, nid_in};
    in_r.insert(make_pair(offset, f_before));
    ptr fresh = ptr(shared_ptr<gdsl::rreil::id>(new ptr_memory_id(nid_in)), vs_finite::zero);
    ptr_set_t ptr_set = ptr_set_t({_nullptr, fresh});
    child_state->assume(&n_in, ptr_set);
    return ptr_set;
  };

  struct field_desc {
    int64_t offset;
    size_t size;
    ptr_set_t ptr_set;
  };

  optional<field_desc> fd_before;
  if(prefix_needed) {
    field_desc fd;
    fd.offset = offset_first.value();
    fd.size = offset - offset_first.value();
    fd.ptr_set = insert_in(fd.offset, fd.size);
    fd_before = fd;
  }

  id_shared_t nid_in = name ? numeric_id::generate(name.value(), offset, size, true) : numeric_id::generate();
  num_var n_in(nid_in);
  ptr_set_t ptr_set_fresh = ptr_set_ct(nid_in);
  /*
   * Todo: assign?!
   */
  child_state->assume(&n_in, ptr_set_fresh);

  /*
   * We do not assign from the input to the output so that there is no equality relation between the input
   * and output. Equality relations result in tests to apply to all equal variables which, in turn, may result
   * in input variables to get their aliasing relations or offsets constrained.
   */
  //  num_expr *ass_e = new num_expr_lin(new num_linear_term(n_in));
  //  cout << "assume " << *n_in << " aliases " << ptr(shared_ptr<gdsl::rreil::id>(new memory_id(0, nid_in)),
  //  vs_finite::zero) << endl;

  optional<field_desc> fd_after;
  if(suffix_needed) {
    field_desc fd;
    fd.offset = offset + size;
    fd.size = (offset_first.value() + total_size.value()) - fd.offset;
    fd.ptr_set = insert_in(fd.offset, fd.size);
    fd_after = fd;
  }

  id_shared_t nid_out = name ? numeric_id::generate(name.value(), offset, size, false) : numeric_id::generate();
  num_var *n_out = new num_var(nid_out);

  //  cout << "contiguous=" << contiguous << endl;

  /*
   * Todo: size > 64?
   */
  if(contiguous && !replacement && replaced.size() > 0 && size <= 64) {
    assert(!fd_before);
    assert(!fd_after);

    optional<num_var *> temp;
    for(size_t i = replaced.size(); i > 0; i--) {
      field f = replaced[i - 1];
      if(temp) {
        num_expr *shift =
          new num_expr_bin(new num_linear_term((*temp)->copy()), SHL, new num_linear_vs(vs_finite::single(f.size)));
        id_shared_t temp_next = numeric_id::generate();
        num_var *temp_var = new num_var(temp_next);
        child_state->assign(temp_var, shift);
        //        cout << "assign " << *temp_var << " = " << *shift << endl;
        num_expr *addition =
          new num_expr_lin(new num_linear_term(1, temp_var->copy(), new num_linear_term(new num_var(f.num_id))));
        child_state->assign(*temp, addition);
        //        cout << "assign " << **temp << " = " << *addition << endl;
        child_state->kill({temp_var});
        delete addition;
        delete temp_var;
        delete shift;
      } else {
        temp = new num_var(f.num_id);
      }
    }
    num_expr *temp_expr = new num_expr_lin(new num_linear_term(*temp));
    child_state->assign(n_out, temp_expr);
    delete temp_expr;
  } else if(!replacement && !fd_before && !fd_after) {
    child_state->assume(n_out, ptr_set_fresh);
  } else {
    if(fd_before) {
      id_shared_t nid_out_before =
        name ? numeric_id::generate(name.value(), fd_before.value().offset, fd_before.value().size, false)
             : numeric_id::generate();
      out_r.insert(make_pair(fd_before.value().offset, field{fd_before.value().size, nid_out_before}));
      num_var n_out_before(nid_out_before);
      child_state->assume(&n_out_before, {badptr});
    }
    child_state->assume(n_out, {badptr});
    if(fd_after) {
//      cout << "Have after! offset: " << fd_after.value().offset << ", size: " << fd_after.value().size << endl;
      id_shared_t nid_out_after =
        name ? numeric_id::generate(name.value(), fd_after.value().offset, fd_after.value().size, false)
             : numeric_id::generate();
      out_r.insert(make_pair(fd_after.value().offset, field{fd_after.value().size, nid_out_after}));
      num_var n_out_after(nid_out_after);
      child_state->assume(&n_out_after, {badptr});
    }
  }
  //  child_state->assign(n_out, ass_e);

  in_r.insert(make_pair(offset, field{size, nid_in}));
  region_t::iterator field_out_it;
  tie(field_out_it, ignore) = out_r.insert(make_pair(offset, field{size, nid_out}));

  delete n_out;
  //  delete ass_e;

  child_state->kill(kill_vars);
  for(num_var *var : kill_vars)
    delete var;

  return field_out_it->second;
}

optional<field> analysis::io_region::retrieve_field(
  numeric_state *child_state, int64_t offset, size_t size, bool replacement, bool handle_conflicts) {
  auto ptr_set_fresh = [](id_shared_t nid_in) {
    ptr _nullptr = ptr(special_ptr::_nullptr, vs_finite::zero);
    ptr fresh = ptr(shared_ptr<gdsl::rreil::id>(new ptr_memory_id(nid_in)), vs_finite::zero);
    return ptr_set_t({_nullptr, fresh});
  };
  return retrieve_field(child_state, offset, size, replacement, handle_conflicts, ptr_set_fresh);
}


/*
 * summary memory state
 */

summary_memory_state *analysis::summary_memory_state::domop(
  bool widening, domain_state *other, size_t current_node, domopper_t domopper) {
  //  cout << "JOIN OF" << endl;
  //  cout << *this << endl;
  //  cout << "WITH" << endl;
  //  cout << *other << endl;
  summary_memory_state *other_casted = dynamic_cast<summary_memory_state *>(other);
  if(is_bottom())
    return other_casted->copy();
  else if(other_casted->is_bottom())
    return copy();

  numeric_state *me_compat;
  numeric_state *other_compat;
  memory_head head_compat;
  tie(ignore, head_compat, me_compat, other_compat) = compat(widening, this, other_casted);

  //  cout << *me_compat << " ^^^JOIN^^^ " << *other_compat << endl;

  summary_memory_state *result = new summary_memory_state(
    sm, warnings, (me_compat->*domopper)(other_compat, current_node), head_compat.input, head_compat.output);
  delete me_compat;
  delete other_compat;
  result->cleanup();
  return result;
}

// summary_memory_state *analysis::summary_memory_state::domop_abstracting(
//  domain_state *other, size_t current_node, domopper_t domopper) {}

std::unique_ptr<managed_temporary> analysis::summary_memory_state::assign_temporary(
  int_t size, std::function<num_expr *(analysis::api::converter &)> cvc) {
  num_var *var = new num_var(numeric_id::generate());
  converter addr_cv(
    size, [&](shared_ptr<gdsl::rreil::id> id, size_t offset, size_t size) { return transLE(id, offset, size); });
  num_expr *addr_expr = cvc(addr_cv);
  child_state->assign(var, addr_expr);
  delete addr_expr;

  return unique_ptr<managed_temporary>(new managed_temporary(*this, var));
}

io_region analysis::summary_memory_state::region_by_id(regions_getter_t getter, id_shared_t id) {
  region_map_t &input_rmap = (input.*getter)();
  region_map_t &output_rmap = (output.*getter)();
  auto id_in_it = input_rmap.find(id);
  if(id_in_it == input_rmap.end()) tie(id_in_it, ignore) = input_rmap.insert(make_pair(id, region_t{}));
  auto id_out_it = output_rmap.find(id);
  if(id_out_it == output_rmap.end()) tie(id_out_it, ignore) = output_rmap.insert(make_pair(id, region_t{}));
  return io_region(
    id_in_it->second, id_out_it->second, getter == &relation::get_regions ? optional<id_shared_t const>(id) : nullopt);
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
  out << endl
      << "}";
}

// region_t &analysis::summary_memory_state::region(id_shared_t id) {
//  auto mapping_it = regions.find(id);
//  if(mapping_it != regions.end()) return mapping_it->second;
//  else {
//    region_map_t::iterator ins_it;
//    tie(ins_it, ignore) = regions.insert(std::make_pair(id, region_t()));
//    return ins_it->second;
//  }
//}

tuple<bool, void *> analysis::summary_memory_state::static_address(id_shared_t id) {
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

summary_memory_state::special_deref_desc_t analysis::summary_memory_state::handle_special_dereference(
  id_shared_t alias_id) {
  bool force_weak = false;
  bool ignore = false;
  std::experimental::optional<summy::rreil::special_ptr_kind> ptr_kind;
  summy::rreil::id_visitor idv;
  idv._([&](special_ptr *sp) { ptr_kind = sp->get_kind(); });
  alias_id->accept(idv);
  if(ptr_kind) {
    switch(ptr_kind.value()) {
      case NULL_PTR: {
        if(warnings) cout << "Warning (load/store): Ignoring possible null pointer dereference" << endl;
        ignore = true;
        break;
      }
      case BAD_PTR: {
        if(warnings) cout << "Warning (load/store): Ignoring possible bad pointer dereference" << endl;
        ignore = true;
        force_weak = true;
        break;
      }
    }
  }
  return special_deref_desc_t{force_weak, ignore};
}

void analysis::summary_memory_state::initialize_static(io_region io, void *address, size_t offset, size_t size) {
  id_shared_t mem_id = transVarReg(io, offset, size);
  if(size > 64) throw string("analysis::summary_memory_state::initialize_static(region_t,void*,size_t): size > 64");

  int64_t sv = 0;
  bool success = sm->read((char *)address + (offset / 8), size / 8, (uint8_t *)&sv);
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

std::tuple<std::set<int64_t>, std::set<int64_t>> analysis::summary_memory_state::overlappings(
  summy::vs_finite *vs, int_t store_size) {
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
  if(overlapping.find(last) == overlapping.end()) non_overlapping.insert(last);

  return make_tuple(overlapping, non_overlapping);
}

void analysis::summary_memory_state::topify(io_region &region, int64_t offset, size_t size) {
  field f = region.retrieve_field(child_state, offset, size, false, true).value();
  num_var nv(f.num_id);
  child_state->kill({&nv});
  /*
   * Todo: Erasing variables is tricky now...
   */
  //    region.erase(field_it->first);
}

optional<id_shared_t> analysis::summary_memory_state::transVarReg(
  io_region io, int64_t offset, size_t size, bool handle_conflict) {
  optional<id_shared_t> r;
  optional<field> f = io.retrieve_field(child_state, offset, size, false, handle_conflict);
  if(f)
    return f.value().num_id;
  else
    return nullopt;
}

id_shared_t analysis::summary_memory_state::transVarReg(io_region io, int64_t offset, size_t size) {
  return transVarReg(io, offset, size, true).value();
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
        if(size_rest == f.size) break;
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
    optional<field> f_opt = io.retrieve_field(child_state, offset, size, false, false);
    if(f_opt)
      return new num_linear_term(new num_var(f_opt.value().num_id));
    else
      return new num_linear_vs(value_set::top);
  } else
    return assemble_fields(fields);
}

num_linear *analysis::summary_memory_state::transLE(
  regions_getter_t rget, id_shared_t var_id, int64_t offset, size_t size) {
  io_region io = region_by_id(rget, var_id);
  return transLEReg(io, offset, size);
}

num_linear *analysis::summary_memory_state::transLE(id_shared_t var_id, int64_t offset, size_t size) {
  //  cout << "transLE(" << *var_id << ", ...)" << endl;
  return transLE(&relation::get_regions, var_id, offset, size);
}

num_linear *analysis::summary_memory_state::transLEInput(id_shared_t var_id, int64_t offset, size_t size) {
  auto id_in_it = input.regions.find(var_id);
  if(id_in_it == input.regions.end()) return new num_linear_vs(value_set::top);
  region_t &region = id_in_it->second;

  vector<field> fields = transLERegFields(region, offset, size);

  if(fields.size() == 0) {
    return new num_linear_vs(value_set::top);
  } else
    return assemble_fields(fields);
}

analysis::summary_memory_state::summary_memory_state(
  shared_ptr<static_memory> sm, bool warnings, numeric_state *child_state, bool start_bottom)
    : memory_state_base(child_state), sm(sm), warnings(warnings) {
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

void analysis::summary_memory_state::check_consistency() {
  if(is_bottom()) {
    assert(input.regions.empty());
    assert(output.regions.empty());
  }
  //    cout << "check_consistency..." << *this << endl;
  auto check_regions = [&](region_map_t &regions_in, region_map_t &regions_out) {
    for(auto &region_it : regions_in) {
      auto region_out_it = regions_out.find(region_it.first);
      assert(region_out_it != regions_out.end());

      //      cout << *region_it.first << endl;
      optional<int64_t> first_free;
      for(auto &f_it : region_it.second) {
        auto f_out_it = region_out_it->second.find(f_it.first);
        assert(f_out_it != region_out_it->second.end());

        if(first_free) assert(f_it.first >= first_free.value());
        first_free = f_it.first + (int64_t)f_it.second.size;
        num_var *nv = new num_var(f_it.second.num_id);
        ptr_set_t aliases = child_state->queryAls(nv);
        delete nv;
        ptr p = ::analysis::unpack_singleton(aliases);
        //          assert(p.id != special_ptr::badptr);
        assert(*p.offset == vs_finite::zero);
      }
    }
  };
  check_regions(input.regions, output.regions);
  check_regions(input.deref, output.deref);

//  id_shared_t sp = id_shared_t(new gdsl::rreil::arch_id("SP"));
//  auto sp_it = output.regions.find(sp);
//  if(sp_it != output.regions.end()) {
//    num_var nv(sp_it->second.at(0).num_id);
//    ptr_set_t aliases_sp = child_state->queryAls(&nv);
//    cout << nv << endl;
//    cout << aliases_sp << endl;
//    cout << *child_state << endl;
//    assert(aliases_sp.size() == 2);
//    for(auto p : aliases_sp)
//      assert(!(*p.id == *special_ptr::badptr));
//  }
}

bool analysis::summary_memory_state::operator>=(const domain_state &other) const {
  summary_memory_state const &other_casted = dynamic_cast<summary_memory_state const &>(other);
  numeric_state *me_compat;
  numeric_state *other_compat;
  bool conflicts;
  tie(conflicts, ignore, me_compat, other_compat) = compat(false, this, &other_casted);
  if(conflicts)
    return false;
  bool result = *me_compat >= *other_compat;
  delete me_compat;
  delete other_compat;
  return result;
}

summary_memory_state *analysis::summary_memory_state::join(domain_state *other, size_t current_node) {
  summary_memory_state *other_casted = (summary_memory_state *)other;
  if(is_bottom()) return other_casted->copy();
  if(other_casted->is_bottom()) return this->copy();
  summary_memory_state *result = domop(false, other, current_node, &numeric_state::join);
  assert(!result->is_bottom());
  return result;
}

summary_memory_state *analysis::summary_memory_state::widen(domain_state *other, size_t current_node) {
  //  cout << "WIDENING OF" << endl;
  //  cout << "THIS: " << *this << endl;
  //  cout << "OTHER: " << *other << endl;
  summary_memory_state *other_casted = (summary_memory_state *)other;
  summary_memory_state *result = domop(true, other, current_node, &numeric_state::widen);
  if(!is_bottom() && !other_casted->is_bottom()) assert(!result->is_bottom());
  return result;
}

summary_memory_state *analysis::summary_memory_state::narrow(domain_state *other, size_t current_node) {
  return domop(false, other, current_node, &numeric_state::narrow);
}

// region_t analysis::summary_memory_state::join_region_aliases(region_t const &r1, region_t const &r2, numeric_state
// *n) {
//  region_t result;
//  cr_merge_region_iterator mri(r1, r2, result);
//  while(mri != cr_merge_region_iterator::end(r1, r2)) {
//    region_pair_desc_t rpd = *mri;
//    field_desc_t const &f_first = rpd.ending_first;
//    num_var *f_first_v = new num_var(f_first.f.num_id);
//    id_shared_t id_r = numeric_id::generate();
//    num_var *v_r = new num_var(id_r);
//    if(!rpd.collision) {
//      if(rpd.ending_last) {
//        field_desc_t const &f_sec = rpd.ending_last.value();
//        num_var *f_sec_v = new num_var(f_sec.f.num_id);
//
//        ptr_set_t f_first_aliases = n->queryAls(f_first_v);
//        ptr_set_t f_sec_aliases = n->queryAls(f_first_v);
//
//        ptr_set_t f_r_aliases = f_first_aliases;
//        f_r_aliases.insert(f_sec_aliases.begin(), f_sec_aliases.end());
//        n->assume(v_r, f_r_aliases);
//
//        result.insert(make_pair(f_first.offset, field{f_first.f.size, id_r}));
//
//        delete f_sec_v;
//      } else {
//        ptr_set_t f_first_aliases = n->queryAls(f_first_v);
//
//        ptr_set_t f_r_aliases = f_first_aliases;
//        n->assume(v_r, f_r_aliases);
//
//        result.insert(make_pair(f_first.offset, field{f_first.f.size, id_r}));
//      }
//    }
//    ++mri;
//  }
//  return result;
//}

void analysis::summary_memory_state::update(gdsl::rreil::assign *assign) {
  if(is_bottom()) return;

  variable *var = assign->get_lhs();
  id_shared_t num_id = transVar(shared_copy(var->get_id()), var->get_offset(), assign->get_size());
  num_var *n_var = new num_var(num_id);
  converter cv(assign->get_size(),
    [&](shared_ptr<gdsl::rreil::id> id, size_t offset, size_t size) { return transLE(id, offset, size); });
  num_expr *n_expr = cv.conv_expr(assign->get_rhs());
  child_state->assign(n_var, n_expr);
  delete n_expr;
  delete n_var;

  cleanup();

  /*
   * Consistency
   */
  assert(!is_bottom());
}

summary_memory_state *analysis::summary_memory_state::copy() const {
  return new summary_memory_state(*this);
}

void summary_memory_state::topify() {
  vector<num_var *> vars;

  auto collect_regions = [&](region_map_t &regions) {
    for(auto regions_it = regions.begin(); regions_it != regions.end(); regions_it++) {
      for(auto &field_mapping : regions_it->second)
        vars.push_back(new num_var(field_mapping.second.num_id));
    }
  };
  collect_regions(output.regions);
  collect_regions(output.deref);

  child_state->kill(vars);

  for(num_var *var : vars)
    delete var;
}

summary_memory_state *analysis::summary_memory_state::start_value(
  shared_ptr<static_memory> sm, bool warnings, numeric_state *start_num) {
  return new summary_memory_state(sm, warnings, start_num, true);
}

summary_memory_state *analysis::summary_memory_state::bottom(
  shared_ptr<static_memory> sm, bool warnings, numeric_state *bottom_num) {
  return new summary_memory_state(sm, warnings, bottom_num, false);
}

void analysis::summary_memory_state::update(gdsl::rreil::load *load) {
  if(is_bottom()) return;

  address *addr = load->get_address();
  auto temp = assign_temporary(addr->get_lin(), addr->get_size());
  vector<num_linear *> lins;
  ptr_set_t aliases = child_state->queryAls(temp->get_var());
  for(auto &alias : aliases) {
    //    cout << "Load Alias: " << *alias.id << "@" << *alias.offset << endl;
    special_deref_desc_t spdd = handle_special_dereference(alias.id);
    if(spdd.ignore) continue;

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

      if(overlapping.size() > 0) return;

      for(auto noo : non_overlapping) {
        if(is_static) initialize_static(io, symbol_address, noo, load->get_size());
        lins.push_back(transLEReg(io, noo, load->get_size()));
      }
    });
    vsv._([&](vs_open *o) { lins.push_back(new num_linear_vs(value_set::top)); });
    vsv._([&](vs_top *t) { lins.push_back(new num_linear_vs(value_set::top)); });
    vs_shared_t offset_bits = *vs_finite::single(8) * alias.offset;
    offset_bits->accept(vsv);
  }

  num_var *lhs =
    new num_var(transVar(shared_copy(load->get_lhs()->get_id()), load->get_lhs()->get_offset(), load->get_size()));
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

  /*
   * Consistency
   */
  assert(!is_bottom());
}

void analysis::summary_memory_state::update_multiple(ptr_set_t aliases, regions_getter_t getter, size_t size,
  updater_t strong, updater_t weak, bool bit_offsets, bool handle_conflicts) {
  //  cout << "update_multiple(" << aliases << ", size: " << size << ")" << endl;
  bool bottom_before = is_bottom();

  bool force_weak = false;
  ptr_set_t aliases_cleaned;
  for(auto &alias : aliases) {
    special_deref_desc_t spdd = handle_special_dereference(alias.id);
    force_weak = force_weak || spdd.force_weak;

    bool is_static = false;
    tie(is_static, ignore) = static_address(alias.id);
    if(is_static) {
      if(warnings) cout << "Warning: Ignoring possible store to static memory" << endl;
      continue;
    }

    if(!spdd.ignore || is_static) aliases_cleaned.insert(alias);
  }

  for(auto &alias : aliases_cleaned) {
    io_region io = region_by_id(getter, alias.id);

    vector<id_shared_t> ids;
    bool singleton = aliases_cleaned.size() == 1;
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
        if(handle_conflicts) topify(io, oo, size);
      for(auto noo : non_overlapping) {
        optional<id_shared_t> next_id = transVarReg(io, noo, size, handle_conflicts);
        if(next_id) ids.push_back(next_id.value());
      }
    });
    vsv._([&](vs_open *o) {
      singleton = false;
      if(!handle_conflicts) return;
      if(warnings) cout << "Warning (store): Ignoring store to an open interval offset" << endl;
      _continue = true;
      /*
       * Todo: The code below is broken.
       */
      //      switch(o->get_open_dir()) {
      //        case UPWARD: {
      //          for(auto field_it = io.out_r.begin(); field_it != io.out_r.end(); field_it++)
      //            if(field_it->first + field_it->second.size > o->get_limit()) topify(io.out_r, field_it->first,
      //            size);
      //          break;
      //        }
      //        case DOWNWARD: {
      //          for(auto field_it = io.out_r.begin(); field_it != io.out_r.end(); field_it++) {
      //            if(field_it->first < o->get_limit())
      //              topify(io.out_r, field_it->first, size);
      //            else if(field_it->first < o->get_limit() + size)
      //              topify(io.out_r, field_it->first, size);
      //          }
      //          break;
      //        }
      //      }

    });
    vsv._([&](vs_top *t) {
      _continue = true;
      singleton = false;
    });
    vs_shared_t offset_bits;
    if(!bit_offsets)
      offset_bits = *vs_finite::single(8) * alias.offset;
    else
      offset_bits = alias.offset;
    offset_bits->accept(vsv);
    /*
     * Erasing from regions is tricky now...
     */
    //    if(region.size() == 0) {
    //      deref.erase(alias.id);
    //      continue;
    //    }
    if(_continue) continue;

    for(auto id : ids) {
      num_var *lhs = new num_var(id);
      if(singleton && !force_weak)
        strong(lhs);
      else
        weak(lhs);
      delete lhs;
    }
  }

  /*
   * Consistency
   */
  assert(bottom_before || !is_bottom());
}

void analysis::summary_memory_state::store(ptr_set_t aliases, size_t size, api::num_expr *rhs) {
  /*
   * Todo: Use bits in als_state and vsd_state
   */
  update_multiple(aliases, &relation::get_deref, size, [&](num_var *lhs) { child_state->assign(lhs, rhs); },
    [&](num_var *lhs) { child_state->weak_assign(lhs, rhs); }, false, true);
}

void analysis::summary_memory_state::update(gdsl::rreil::store *store) {
  if(is_bottom()) return;

  address *addr = store->get_address();
  auto temp = assign_temporary(addr->get_lin(), addr->get_size());
  ptr_set_t aliases = child_state->queryAls(temp->get_var());

  converter rhs_cv(store->get_size(),
    [&](shared_ptr<gdsl::rreil::id> id, size_t offset, size_t size) { return transLE(id, offset, size); });
  num_expr *rhs = rhs_cv.conv_expr(store->get_rhs());
  this->store(aliases, store->get_size(), rhs);
  delete rhs;

  cleanup();

  /*
   * Consistency
   */
  assert(!is_bottom());
}

void analysis::summary_memory_state::assume(gdsl::rreil::sexpr *cond) {
  converter cv(
    0, [&](shared_ptr<gdsl::rreil::id> id, size_t offset, size_t size) { return transLE(id, offset, size); });
  expr_cmp_result_t ecr = cv.conv_expr_cmp(cond);
  child_state->assume(ecr.primary);
  for(auto add : ecr.additional)
    child_state->assume(add);
  ecr.free();
  if(child_state->is_bottom()) {
    bottomify();
    return;
  }

  /*
   * Todo: Is this necessary?
   */
  unique_ptr<managed_temporary> temp = assign_temporary(cond, 1);
  vs_shared_t value = child_state->queryVal(temp->get_var());
  if(*value == vs_finite::_false) bottomify();
}

void analysis::summary_memory_state::assume_not(gdsl::rreil::sexpr *cond) {
  converter cv(
    0, [&](shared_ptr<gdsl::rreil::id> id, size_t offset, size_t size) { return transLE(id, offset, size); });
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
  if(child_state->is_bottom()) {
    bottomify();
    return;
  }

  /*
   * Todo: Is this necessary?
   */
  unique_ptr<managed_temporary> temp = assign_temporary(cond, 1);
  vs_shared_t value = child_state->queryVal(temp->get_var());
  if(*value == vs_finite::_true) bottomify();
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

void analysis::summary_memory_state::rename() {
  //  summary_memory_state *old = this->copy();

  /*
   * First, we build up a map that maps all id objects
   * to occurrences of shared pointers referring to them.
   */
  std::map<gdsl::rreil::id *, std::set<analysis::id_shared_t *>> id_map;
  child_state->collect_ids(id_map);
  auto _inner = [&](auto &regions) {
    auto region_it = regions.begin();
    while(region_it != regions.end()) {
      id_map[region_it->first.get()].insert((analysis::id_shared_t *)&region_it->first);
      auto field_it = region_it->second.begin();
      while(field_it != region_it->second.end()) {
        id_map[field_it->second.num_id.get()].insert((analysis::id_shared_t *)&field_it->second.num_id);
        field_it++;
      }
      region_it++;
    }
  };
  _inner(input.regions);
  _inner(input.deref);
  _inner(output.regions);
  _inner(output.deref);

  /*
   * Next, we build a mapping that maps each numeric id counter to its repective
   * id object. This way, we sort the ids by counter value. This is important
   * because we have to keep the order of the variables as we rename (i.e.
   * replace) them since we do the renaming without rebuilding the data
   * structures the variables live in. This approach is kind of daredevil ;-).
   *
   * We also have to rename memory ids. A memory id contains a numeric id. We
   * therefore also add the set of memory ids containing the numeric id with the
   * respective counter to the mapping. This way we can replace the memory ids
   * together with the numeric ids.
   */
  struct rev_id {
    optional<gdsl::rreil::id *> _id;
    set<ptr_memory_id *> memory_ids;
  };
  std::map<size_t, rev_id> rev_map;
  for(auto &id_map_it : id_map) {
    summy::rreil::id_visitor idv;
    idv._([&](numeric_id *nid) { rev_map[nid->get_counter()]._id = nid; });
    idv._([&](ptr_memory_id *mid) {
      bool found = false;
      summy::rreil::id_visitor inner_idv;
      inner_idv._([&](numeric_id *nid) {
        rev_map[nid->get_counter()].memory_ids.insert(mid);
        found = true;
      });
      mid->get_id()->accept(inner_idv);
      assert(found);
    });
    id_map_it.first->accept(idv);
  }

  /*
   * Finally, we replace the respective numeric variables. Memory ids are
   * rebuild using the fresh variables.
   */
  for(auto &rev_it : rev_map) {
    optional<id_shared_t> _fresh;
    auto fresh = [&]() {
      if(!_fresh) _fresh = numeric_id::generate();
      return _fresh.value();
    };
    if(rev_it.second._id)
      for(analysis::id_shared_t *instance : id_map.at(rev_it.second._id.value()))
        *instance = fresh();
    for(ptr_memory_id *mid : rev_it.second.memory_ids) {
      id_shared_t memory_fresh = make_shared<ptr_memory_id>(fresh());
      for(analysis::id_shared_t *instance : id_map.at(mid))
        *instance = memory_fresh;
    }
  }

  /*
   * Enable this for debugging!
   */
  //  assert(*old == *this);
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
    if(known_ids.find(next) != known_ids.end()) continue;

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
  return assign_temporary(size, [&](converter &cv) { return cv.conv_expr(e); });
}

std::unique_ptr<managed_temporary> analysis::summary_memory_state::assign_temporary(
  gdsl::rreil::linear *l, int_t size) {
  return assign_temporary(size, [&](converter &cv) { return cv.conv_expr(l); });
}

std::unique_ptr<managed_temporary> analysis::summary_memory_state::assign_temporary(
  gdsl::rreil::sexpr *se, int_t size) {
  return assign_temporary(size, [&](converter &cv) { return cv.conv_expr(se); });
}

summy::vs_shared_t analysis::summary_memory_state::queryVal(gdsl::rreil::linear *l, size_t size) {
  converter cv(
    size, [&](shared_ptr<gdsl::rreil::id> id, size_t offset, size_t size) { return transLE(id, offset, size); });
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

ptr_set_t analysis::summary_memory_state::queryAls(gdsl::rreil::address *a) {
  if(is_bottom()) return ptr_set_t{};
  auto temp = assign_temporary(a->get_lin(), a->get_size());
  ptr_set_t aliases = child_state->queryAls(temp->get_var());
  return aliases;
}

ptr_set_t analysis::summary_memory_state::queryAls(api::num_var *v) {
  ptr_set_t aliases = child_state->queryAls(v);
  return aliases;
}

const region_t &analysis::summary_memory_state::query_region_output(id_shared_t id) {
  return output.regions[id];
}

const region_t &analysis::summary_memory_state::query_deref_output(id_shared_t id) {
  return output.deref[id];
}
