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
#include <summy/rreil/id/sm_id.h>
#include <summy/value_set/vs_finite.h>
#include <experimental/optional>
#include <string>

using gdsl::rreil::id;
using std::experimental::optional;
using summy::rreil::sm_id;
using summy::value_set;
using summy::vs_finite;

using namespace std;
using namespace analysis;
using namespace analysis::api;

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
  //  from = from->copy();
  optional<ptr_set_t> aliases_from;

  //  cout << *from << endl;

  auto from_io = from->region_by_id(&relation::get_regions, base);
  optional<ptr_set_t> aliases_current;
  for(size_t i = 0; i < path.size(); ++i) {
    //      if(from_reg_it == from->output.deref.end())
    //        break;
    size_t offset = path[i].offset;
    size_t size = path[i].size;
    auto field_it = from_io.out_r.find(offset);
    if(field_it == from_io.out_r.end()) assert(false);
    field f = field_it->second;
    if(f.size != size) f = from_io.insert(from->child_state, offset, size, true);
    num_var f_var = num_var(f.num_id);
    aliases_from = from->queryAls(&f_var);
    /*
     * Todo: Multiple aliases!
     */
    cout << aliases_from.value() << endl;
    //    ptr singleton = unpack_singleton(aliases_from.value());
    //    /*
    //     * Todo: ...
    //     */
    //    assert(*singleton.offset == vs_finite::zero);
    //    from_io = from->region_by_id(&relation::get_deref, singleton.id);
  }

  return aliases_from.value();
}

std::tuple<ptr_set_t, ptr_set_t> analysis::mempath::split(ptr_set_t aliases) {
  ptr_set_t aliases_immediate;
  ptr_set_t aliases_symbolic;
  for(auto &alias : aliases) {
    summy::rreil::id_visitor idv;
    idv._([&](sm_id *sid) { aliases_immediate.insert(alias); });
    idv._default([&](id *_id) {
      aliases_symbolic.insert(alias);
    });
    alias.id->accept(idv);
    // Take over aliases we can propagate
  }
  return make_tuple(aliases_immediate, aliases_symbolic);
}

void analysis::mempath::propagate(ptr_set_t aliases_from_immediate, summary_memory_state *to) const {

  if(aliases_from_immediate.size() > 0) {
    // propagate
    assert(to->input.regions.find(base) == to->input.regions.end());
    assert(path.size() > 0);

    optional<field> f;
    io_region io = to->region_by_id(&relation::get_regions, base);

    for(size_t i = 0; i < path.size(); ++i) {
      f = io.insert(to->child_state, path[i].offset, path[i].size, false);
      num_var f_id_nv = num_var(f.value().num_id);
      ptr_set_t aliases_f = to->child_state->queryAls(&f_id_nv);
      ptr p = unpack_singleton(aliases_f);
      cout << p << endl;
      assert(*p.offset == vs_finite::zero);
      io = to->region_by_id(&relation::get_deref, p.id);
    }

    if(f) {
      num_var f_var(f->num_id);
      to->child_state->assign(&f_var, aliases_from_immediate);
    }
  }

  //  cout << *to << endl;
}

std::experimental::optional<set<mempath>> analysis::mempath::propagate(
  summary_memory_state *from, summary_memory_state *to) const {
  ptr_set_t aliases_from = resolve(from);

  ptr_set_t aliases_from_immediate;
  ptr_set_t aliases_from_symbolic;
  tie(aliases_from_immediate, aliases_from_symbolic) = split(aliases_from);

  propagate(aliases_from_immediate, to);

  if(aliases_from_symbolic.size() > 0) {
    id_set_t aliases_from_symbolic_ids;
    for(auto ptr : aliases_from_symbolic)
      aliases_from_symbolic_ids.insert(ptr.id);
    /*
     * Todo: What if offsets are != zero?!
     */
    return from_aliases(aliases_from_symbolic_ids, from);
  } else
    return experimental::nullopt;
}

std::set<mempath> analysis::mempath::from_aliases(api::id_set_t aliases, summary_memory_state *state) {
  /*
   * Todo: id_set, not ptr_set
   */
  set<mempath> result;
  for(auto &alias : aliases) {
    bool found = false;
    for(region_map_t::iterator region_it = state->input.regions.begin(); region_it != state->input.regions.end();
        region_it++) {
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
        result.insert(mempath(region_it->first, steps));
        found = true;
        break;
      }
    }
    if(!found) cout << "analysis::mempath::from_pointers() - Warning: Unable to find pointer." << endl;
  }
  return result;
}

std::ostream &analysis::operator<<(std::ostream &out, const mempath &_this) {
  out << *_this.base << "/" << _this.path[0].offset << ":" << _this.path[0].size;
  for(size_t i = 1; i < _this.path.size(); ++i)
    out << "->" << _this.path[i].offset << ":" << _this.path[i].size;
  return out;
}
