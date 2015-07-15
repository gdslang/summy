/*
 * sms_op.cpp
 *
 *  Created on: Jul 15, 2015
 *      Author: Julian Kranz
 */

#include <assert.h>
#include <summy/analysis/domains/sms_op.h>
#include <summy/analysis/domain_state.h>
#include <summy/analysis/domains/api/numeric/num_expr.h>
#include <summy/analysis/domains/api/numeric/num_linear.h>
#include <summy/analysis/domains/api/numeric/num_var.h>
#include <summy/analysis/domains/api/ptr_set.h>
#include <summy/analysis/domains/summary_memory_state.h>
#include <summy/value_set/value_set.h>
#include <summy/value_set/vs_finite.h>
#include <cppgdsl/rreil/rreil.h>
#include <map>
#include <algorithm>

using namespace std;
using namespace analysis;
using namespace analysis::api;
using namespace summy;

summary_memory_state * ::analysis::apply_summary(summary_memory_state *caller, summary_memory_state *summary) {
  summary_memory_state *return_site = caller->copy();

  /*
   * We need a copy in order to add new variables for joined regions addressing unexpected aliasing
   * situations
   */
  //  summary = summary->copy();

  /*
   * Each of the variables of the summary maps to a number of variables in the caller at
   * certain offsets. The mapping is established from the structure of the memory
   * of the summary.
   */
  map<id_shared_t, ptr_set_t, id_less_no_version> ptr_map;

  typedef std::set<id_shared_t, id_less_no_version> alias_queue_t;
  alias_queue_t ptr_worklist;

  auto offsets_bytes_to_bits_base = [&](int64_t base, ptr_set_t const &region_keys_c) {
    vs_shared_t vs_f_offset_s = vs_finite::single(base);
    ptr_set_t region_keys_c_offset_bits;
    for(auto &_ptr : region_keys_c) {
      vs_shared_t offset_new = *(*vs_finite::single(8) * _ptr.offset) + vs_f_offset_s;
      region_keys_c_offset_bits.insert(ptr(_ptr.id, offset_new));
    }
    return region_keys_c_offset_bits;
  };

  auto build_pmap_region = [&](
    id_shared_t region_key_summary, ptr_set_t const &region_keys_c, summary_memory_state::regions_getter_t rgetter) {
    auto next_ids = (summary->input.*rgetter)().find(region_key_summary);
    if(next_ids == (summary->input.*rgetter)().end()) return;

    region_t &region_s = next_ids->second;

    for(auto &field_mapping_s : region_s) {
      field &f_s = field_mapping_s.second;
      num_var *nv_field_s = new num_var(f_s.num_id);
      ptr_set_t aliases_fld_s = summary->child_state->queryAls(nv_field_s);

      assert(aliases_fld_s.size() <= 1);

      ptr_set_t region_keys_c_offset_bits = offsets_bytes_to_bits_base(field_mapping_s.first, region_keys_c);

      /*
       * Todo: Warning if an alias is found in the summary plus this alias has a region in the deref map
       * and no alias is found in 'c'
       * Todo: What about an alias in 'me' with no alias in the summary? We should somehow remove the alias in 'c'
       * then
       */

      ptr_set_t aliases_fld_c;

      summary_memory_state::updater_t record_aliases = [&](num_var *nv_flc_c) {
        //          cout << "New mapping in region " << *next_me << " from " << *f_s.num_id << " to " << *id_me <<
        //          endl;
        //        variable_mapping[f_s.num_id].insert(ptr(nv_me->get_id(), vs_finite::zero));
        ptr_set_t aliases_fld_c_next = return_site->child_state->queryAls(nv_flc_c);
        /*
         * Record new aliases...
         */
        aliases_fld_c.insert(aliases_fld_c_next.begin(), aliases_fld_c_next.end());
      };

      //      cout << region_keys_c_offset_bits << ":" << f_s.size << endl;

      /*
       * We do not 'handle_conflicts' here in order to deal with unexpected aliasing situations
       */
      return_site->update_multiple(
        region_keys_c_offset_bits, rgetter, f_s.size, record_aliases, record_aliases, true, false);

      delete nv_field_s;

      for(auto &p_s : aliases_fld_s) {
        ptr_set_t &aliases_c = ptr_map[p_s.id];
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
      ptr_set_t const &region_keys_c = ptr_map.at(region_key_summary);

      build_pmap_region(region_key_summary, region_keys_c, &relation::get_deref);
    }

  } while(!ptr_worklist.empty());

  /*
   * Having built the relation between variable and pointers names of the caller state and the summary,
   * we apply the summary by updating the caller using the summary state.
   */

  /*
   * Application
   */

  /*
   * We build a ptr_map_rev and a conflict region map in order to deal with unexpected aliasing situations
   */
  map<id_shared_t, id_set_t, id_less_no_version> ptr_map_rev;
  for(auto &ptr_mapping : ptr_map)
    for(auto &p : ptr_mapping.second)
      if(summary->output.deref.find(ptr_mapping.first) != summary->output.deref.end())
        ptr_map_rev[p.id].insert(ptr_mapping.first);

  bool dirty = false;
  map<id_shared_t, region_t, id_less_no_version> conflict_regions;
  for(auto &rev_mapping : ptr_map_rev) {
    id_set_t &ptrs_s = rev_mapping.second;
    if(ptrs_s.size() > 1) {
      dirty = true;
      /*
       * Handling of unexpected aliases...
       */
      //      auto ptrs_it = ptrs_s.begin();
      //      region_t cr = summary->output.deref.at(*ptrs_it);
      //      while(++ptrs_it != ptrs_s.end())
      //        cr = join_region_aliases(cr, summary->output.deref.at(*ptrs_it), summary->child_state);
      //      conflict_regions[rev_mapping.first] = cr;
    }
  }
  if(dirty) {
    return_site->topify();
    return return_site;
  }

  num_expr *_top = new num_expr_lin(new num_linear_vs(value_set::top));
  auto process_region =
    [&](summary_memory_state::regions_getter_t getter, ptr_set_t &region_aliases_c, region_t &region) {
      for(auto &field_mapping_s : region) {
        field &f_s = field_mapping_s.second;
        //      id_shared_t id_me = me_copy->transVar(region_key, field_mapping_s.first, f_s.size);

        //      cout << "    " << *id_me << endl;

        num_var *nv_s = new num_var(f_s.num_id);
        ptr_set_t aliases_s = summary->child_state->queryAls(nv_s);
        delete nv_s;

        ptr_set_t aliases_c;
        for(auto &alias_s : aliases_s) {
          auto aliases_mapped_it = ptr_map.find(alias_s.id);
          ptr_set_t const &aliases_c_next = aliases_mapped_it->second;
          //        assert(aliases_mapped_it != alias_map.end() && aliases_me_ptr.size() > 0);
          //        cout << "search result for " << *_ptr.id << ": " << (aliases_mapped_it != alias_map.end()) << endl;
          if(aliases_mapped_it != ptr_map.end())
            for(auto alias_c_next : aliases_c_next)
              aliases_c.insert(ptr(alias_c_next.id, *alias_c_next.offset + alias_s.offset));
        }

        summary_memory_state::updater_t strong = [&](api::num_var *nv_fld_c) {
          //        cout << "strong for " << *nv_me << ": " << aliases_me << endl;
          return_site->child_state->assign(nv_fld_c, _top);
          return_site->child_state->assume(nv_fld_c, aliases_c);
        };
        summary_memory_state::updater_t weak = [&](api::num_var *nv_fld_c) {
          //        cout << "weak for " << *nv_me << ": " << aliases_me << endl;
          ptr_set_t aliases_joined_c = return_site->child_state->queryAls(nv_fld_c);
          return_site->child_state->assign(nv_fld_c, _top);
          aliases_joined_c.insert(aliases_c.begin(), aliases_c.end());
          return_site->child_state->assume(nv_fld_c, aliases_joined_c);
        };
        ptr_set_t region_aliases_c_offset_bits = offsets_bytes_to_bits_base(field_mapping_s.first, region_aliases_c);

        //      cout << "~~~" << region_aliases_c_offset_bits << ":" << f_s.size << endl;
        return_site->update_multiple(region_aliases_c_offset_bits, getter, f_s.size, strong, weak, true, true);
      }
    };

  for(auto &region_mapping_so : summary->output.regions) {
    id_shared_t region_key = region_mapping_so.first;
    ptr_set_t region_aliases_c = ptr_set_t({ptr(region_key, vs_finite::zero)});
    process_region(&relation::get_regions, region_aliases_c, region_mapping_so.second);
  }

  for(auto &deref_mapping_so : summary->output.deref) {
    id_shared_t region_key = deref_mapping_so.first;
    ptr_set_t &region_aliases_c = ptr_map.at(region_key);
    process_region(&relation::get_deref, region_aliases_c, deref_mapping_so.second);
  }
  delete _top;

  num_vars *_vars = return_site->vars_relations();
  return_site->project(_vars);
  delete _vars;

  //  cout << *return_site << endl;
  //  delete summary;

  return return_site;
}
