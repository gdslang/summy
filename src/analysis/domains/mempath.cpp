/*
 * mempath.cpp
 *
 *  Created on: Oct 29, 2015
 *      Author: Julian Kranz
 */

#include <assert.h>
#include <cppgdsl/rreil/id/id.h>
#include <summy/analysis/domains/api/numeric/num_var.h>
#include <summy/analysis/domains/mempath.h>
#include <summy/analysis/domains/ptr_set.h>
#include <summy/analysis/domains/sms_op.h>
#include <summy/rreil/id/memory_id.h>
#include <summy/rreil/id/sm_id.h>
#include <summy/value_set/value_set_visitor.h>
#include <summy/value_set/vs_finite.h>
#include <experimental/optional>
#include <string>

using gdsl::rreil::id;
using summy::rreil::ptr_memory_id;
using summy::rreil::sm_id;
using summy::value_set;
using summy::value_set_visitor;
using summy::vs_finite;

using namespace std;
using namespace std::experimental;
using namespace analysis;
using namespace analysis::api;
using namespace summy::rreil;

bool mempath::step::operator<(const step &other) const {
  if(offset > other.offset)
    return true;
  else if(offset < other.offset)
    return false;
  else
    return size < other.size;
}

mempath::mempath(std::shared_ptr<gdsl::rreil::id> base, std::vector<step> path) : base(base), path(path) {
  if(path.size() < 1) throw string("Invalid path :-/");
}

int mempath::compare_to(const mempath &other) const {
  if(*base < *other.base)
    return -1;
  else if(*other.base < *base)
    return 1;
  else {
    if(path.size() < other.path.size())
      return -1;
    else if(path.size() > other.path.size())
      return 1;
    else
      for(int i = 0; i < path.size(); ++i) {
        if(path[i] < other.path[i])
          return -1;
        else if(other.path[i] < path[i])
          return 1;
      }
  }
  return 0;
}

bool mempath::operator<(const mempath &other) const {
  return compare_to(other) == -1;
}

bool mempath::operator==(const mempath &other) const {
  return compare_to(other) == 0;
}

ptr_set_t analysis::mempath::resolve(summary_memory_state *from) const {
  ptr_set_t aliases_from;

  assert(path.size() > 0);

  struct work_item {
    size_t index;
    ptr_set_t aliases;
  };
  vector<work_item> work;

  auto step = [&](size_t index, io_region &from_io, int64_t offset_alias) {
    int64_t offset = path[index].offset + offset_alias;
    size_t size = path[index].size;
    auto field_it = from_io.out_r.find(offset);
    field f;
    if(field_it == from_io.out_r.end())
      f = from_io.retrieve_field(from->child_state, offset, size, false, true).f.value();
    else
      f = field_it->second;
    if(f.size != size) f = from_io.retrieve_field(from->child_state, offset, size, true, true).f.value();
    num_var f_var = num_var(f.num_id);
    ptr_set_t aliases = from->queryAls(&f_var);
    if(index + 1 >= path.size())
      aliases_from.insert(aliases.begin(), aliases.end());
    else
      work.push_back({index + 1, aliases});
  };

  auto from_io = from->region_by_id(&relation::get_regions, base);
  step(0, from_io, 0);

  while(work.size() > 0) {
    work_item wi = work.back();
    work.pop_back();
    for(auto &alias : wi.aliases) {
      optional<int64_t> offset;
      value_set_visitor vsv;
      vsv._([&](vs_finite *vsf) {
        if(vsf->is_singleton()) offset = *vsf->get_elements().begin();
      });
      vsv._default([&](value_set *vs) {
        /*
         * Warning?
         */
      });
      alias.offset->accept(vsv);
      summy::rreil::id_visitor idv;
      auto valid_ptr = [&]() {
        if(offset) {
          from_io = from->region_by_id(&relation::get_deref, alias.id);
          step(wi.index, from_io, offset.value());
        }
      };
      idv._([&](ptr_memory_id *mid) { valid_ptr(); });
      idv._([&](allocation_memory_id *mid) { valid_ptr(); });
      idv._default([&](id *_id) {
        /*
         * Warning?
         */
      });
      alias.id->accept(idv);
    }
  }

  return aliases_from;
}

std::tuple<ptr_set_t, ptr_set_t> analysis::mempath::split(ptr_set_t aliases) {
  ptr_set_t aliases_immediate;
  ptr_set_t aliases_symbolic;
  for(auto &alias : aliases) {
    summy::rreil::id_visitor idv;
    idv._([&](sm_id *sid) { aliases_immediate.insert(alias); });
    idv._default([&](id *_id) { aliases_symbolic.insert(alias); });
    alias.id->accept(idv);
    // Take over aliases we can propagate
  }
  return make_tuple(aliases_immediate, aliases_symbolic);
}

void analysis::mempath::propagate(ptr_set_t aliases_from_immediate, summary_memory_state *to) const {
  if(aliases_from_immediate.size() == 0) return;

  assert(path.size() > 0);

  auto step = [&](size_t index, io_region &io) -> optional<id_shared_t> {
    int64_t offset = path[index].offset;
    size_t size = path[index].size;
    auto field_it = io.out_r.find(offset);
    field f;
    if(field_it == io.out_r.end())
      f = io.retrieve_field(to->child_state, offset, size, false, true).f.value();
    else if(f.size != size)
      return nullopt;
    else
      f = field_it->second;
    return f.num_id;
  };

  auto warn_bad = [&]() { cout << "Warning (analysis::mempath::propagate): Conflicts while propagating pointers."; };

  io_region io = to->region_by_id(&relation::get_regions, base);
  optional<id_shared_t> opt_id = step(0, io);
  if(!opt_id) {
    warn_bad();
    return;
  }

  for(size_t i = 1; i < path.size(); ++i) {
    num_var f_var(opt_id.value());
    ptr_set_t aliases = to->queryAls(&f_var);
    ptr _ptr = unpack_singleton(aliases);
    assert(*_ptr.offset == vs_finite::zero);
    io = to->region_by_id(&relation::get_deref, _ptr.id);
    opt_id = step(i, io);
    if(!opt_id) {
      warn_bad();
      return;
    }
  }

  num_var f_var(opt_id.value());
  to->child_state->assign(&f_var, aliases_from_immediate);
}

std::experimental::optional<set<mempath>> analysis::mempath::propagate(
  summary_memory_state *from, summary_memory_state *to) const {
  //  cout << "propagate " << *this << " from" << endl;
  //  cout << *from << endl;

  ptr_set_t aliases_from = resolve(from);

  ptr_set_t aliases_from_immediate;
  ptr_set_t aliases_from_symbolic;
  tie(aliases_from_immediate, aliases_from_symbolic) = split(aliases_from);

  propagate(aliases_from_immediate, to);

  if(aliases_from_symbolic.size() > 0) {
    id_set_t aliases_from_symbolic_ids;
    for(auto ptr : aliases_from_symbolic)
      aliases_from_symbolic_ids.insert(ptr.id);
    return from_aliases(aliases_from_symbolic_ids, from);
  } else
    return experimental::nullopt;
}

std::set<mempath> analysis::mempath::from_aliases(api::id_set_t aliases, summary_memory_state *state) {
  //  cout << "std::set<mempath> analysis::mempath::from_aliases()" << endl;
  set<mempath> result;
  for(auto &alias : aliases) {
    if(*alias == *special_ptr::_nullptr || *alias == *special_ptr::badptr) continue;
    bool found = false;
    auto for_region_mapping = [&](region_map_t::iterator region_it) {
      //      cout << "region: " << *region_it->first << endl;
      vector<mempath::step> steps;
      function<bool(region_map_t::iterator &)> handle_region;
      handle_region = [&](region_map_t::iterator &region_it) {
        for(auto &field_it : region_it->second) {
          steps.push_back(step(field_it.first, field_it.second.size));
          num_var field_nv = num_var(field_it.second.num_id);
          ptr_set_t aliases_field = state->queryAls(&field_nv);
          struct ptr alias_field = unpack_singleton(aliases_field);
          assert(*alias_field.offset == vs_finite::zero);
          if(*alias == *alias_field.id) return true;
          auto deref_region_it = state->input.deref.find(alias_field.id);
          if(deref_region_it != state->input.deref.end())
            if(handle_region(deref_region_it)) return true;
          steps.pop_back();
        }
        return false;
      };
      if(handle_region(region_it)) {
        //        cout << mempath(region_it->first, steps) << endl;
        result.insert(mempath(region_it->first, steps));
        found = true;
        return true;
      }
      return false;
    };
    for(region_map_t::iterator region_it = state->input.regions.begin(); region_it != state->input.regions.end();
        region_it++)
      if(for_region_mapping(region_it)) break;
    //    if(!found)
    //      for(region_map_t::iterator region_it = state->input.deref.begin(); region_it != state->input.deref.end();
    //          region_it++) {
    //        if(*alias == *region_it->first) {
    //          result.insert(mempath(region_it->first, vector<mempath::step> {step(0, 64)}));
    //          found = true;
    //          break;
    //        }
    //        id_visitor idv;
    //        bool ptr_mem = false;
    //        idv._([&](ptr_memory_id *pid) { ptr_mem = true; });
    //        region_it->first->accept(idv);
    //        if(!ptr_mem)
    //          if(for_region_mapping(region_it)) break;
    //      }
    if(!found) {
      cout << "analysis::mempath::from_pointers() - Warning: Unable to find pointer." << endl;
    }
  }
  return result;
}

std::ostream &analysis::operator<<(std::ostream &out, const mempath &_this) {
  out << *_this.base << "/" << _this.path[0].offset << ":" << _this.path[0].size;
  for(size_t i = 1; i < _this.path.size(); ++i)
    out << "->" << _this.path[i].offset << ":" << _this.path[i].size;
  return out;
}
