/*
 * mempath.cpp
 *
 *  Created on: Oct 29, 2015
 *      Author: Julian Kranz
 */

#include <assert.h>
#include <summy/analysis/domains/api/numeric/num_var.h>
#include <summy/analysis/domains/mempath.h>
#include <summy/analysis/domains/ptr_set.h>
#include <summy/analysis/domains/sms_op.h>
#include <summy/value_set/vs_finite.h>
#include <experimental/optional>
#include <string>

using std::experimental::optional;
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

bool analysis::mempath::propagate(summary_memory_state *from, summary_memory_state *to) {
  optional<ptr_set_t> aliases_from;

  auto from_reg_it = from->output.regions.find(base);
  if(from_reg_it != from->output.regions.end()) {
    optional<ptr_set_t> aliases_current;
    for (size_t i = 0; i < path.size(); ++i) {
//      if(from_reg_it == from->output.deref.end())
//        break;
      size_t offset = path[i].offset;
      size_t size = path[i].size;
      auto field_it = from_reg_it->second.find(offset);
      if(field_it == from_reg_it->second.end())
        return true;
      field &f = field_it->second;
      if(f.size != size)
        return true;
      num_var f_var = num_var(f.num_id);
      aliases_from = from->queryAls(&f_var);
      /*
       * Todo: Multiple aliases!
       */
      ptr singleton = unpack_singleton(aliases_from.value());
      /*
       * Todo: ...
       */
      assert(*singleton.offset == vs_finite::zero);
      from_reg_it = from->output.deref.find(singleton.id);
      if(from_reg_it == from->output.deref.end())
        return true;
    }
  }

  if(!aliases_from)
    return true;

  ptr_set_t aliases_explicit;
  bool result = false;
  for(auto &alias : aliases_from.value()) {
    //Take over aliases we can propagate
  }
  if(aliases_explicit.size() > 0) {
    // propagate
  }
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
    if(!found)
      cout << "analysis::mempath::from_pointers() - Warning: Unable to find pointer." << endl;
  }
  return result;
}

std::ostream &analysis::operator<<(std::ostream &out, const mempath &_this) {
  out << *_this.base << "/" << _this.path[0].offset << ":" << _this.path[0].size;
  for(size_t i = 1; i < _this.path.size(); ++i)
    out << "->" << _this.path[i].offset << ":" << _this.path[i].size;
  return out;
}
