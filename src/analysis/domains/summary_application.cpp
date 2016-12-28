/*
 * summary_application.cpp
 *
 *  Created on: Jun 28, 2016
 *      Author: Julian Kranz
 */

#include <algorithm>
#include <assert.h>
#include <experimental/optional>
#include <summy/analysis/domains/api/numeric/num_expr.h>
#include <summy/analysis/domains/api/numeric/num_linear.h>
#include <summy/analysis/domains/api/numeric/num_var.h>
#include <summy/analysis/domains/numeric/vsd_state.h>
#include <summy/analysis/domains/summary_application.h>
#include <summy/analysis/domains/summary_memory_state.h>
#include <summy/analysis/domains/util.h>
#include <summy/rreil/id/special_ptr.h>
#include <summy/value_set/value_set.h>
#include <summy/value_set/value_set_visitor.h>
#include <summy/value_set/vs_finite.h>

using analysis::api::num_expr;
using analysis::api::num_expr_lin;
using analysis::api::num_linear_vs;
using analysis::api::num_var;
using analysis::api::num_vars;
using analysis::value_sets::vsd_state;
using std::experimental::optional;
using std::experimental::nullopt;
using summy::value_set_visitor;
using summy::vs_finite;
using summy::vs_shared_t;

using namespace summy;
using namespace analysis::api;
using namespace summy::rreil;
using namespace analysis;

using namespace std;

ptr_set_t analysis::summary_application::offsets_bytes_to_bits_base(
  int64_t base, const ptr_set_t &region_keys_c) {
  vs_shared_t vs_f_offset_s = vs_finite::single(base);
  ptr_set_t region_keys_c_offset_bits;
  for(auto &_ptr : region_keys_c) {
    vs_shared_t offset_new = *(*vs_finite::single(8) * _ptr.offset) + vs_f_offset_s;
    region_keys_c_offset_bits.insert(ptr(_ptr.id, offset_new));
  }
  return region_keys_c_offset_bits;
}

void analysis::summary_application::build_pmap_region(id_shared_t region_key_summary,
  const ptr_set_t &region_keys_c, summary_memory_state::regions_getter_t rgetter) {
  summary_memory_state *return_site = *this->return_site;

  auto next_ids = (summary->input.*rgetter)().find(region_key_summary);
  if(next_ids == (summary->input.*rgetter)().end()) return;

  region_t &region_s = next_ids->second;

  for(auto &field_mapping_s : region_s) {
    field &f_s = field_mapping_s.second;
    num_var *nv_field_s = new num_var(f_s.num_id);
    ptr_set_t aliases_fld_s = summary->child_state->queryAls(nv_field_s);

    ptr_set_t region_keys_c_offset_bits =
      offsets_bytes_to_bits_base(field_mapping_s.first, region_keys_c);

    /*
     * Todo: Warning if an alias is found in the summary plus this alias has a region in the deref
     * map
     * and no alias is found in 'c'
     * Todo: What about an alias in 'me' with no alias in the summary? We should somehow remove the
     * alias in 'c'
     * then
     */

    ptr_set_t aliases_fld_c;

    summary_memory_state::updater_t record_aliases = [&](num_var *nv_flc_c) {
      //          cout << "New mapping in region " << *next_me << " from " << *f_s.num_id << " to "
      //          << *id_me <<
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

    auto p_s = unpack_singleton(aliases_fld_s);
    if(!(*p_s.id == *special_ptr::badptr)) {
      ptr_set_t &aliases_c = ptr_map[p_s.id];
      if(!includes(
           aliases_c.begin(), aliases_c.end(), aliases_fld_c.begin(), aliases_fld_c.end())) {
        aliases_c.insert(aliases_fld_c.begin(), aliases_fld_c.end());
        ptr_worklist.insert(p_s.id);
      }
    }
  }
}

analysis::summary_application::summary_application(
  summary_memory_state *caller, summary_memory_state *summary)
    : caller(caller), summary(summary) {
  this->return_site = nullopt;
}

void analysis::summary_application::process_region(
  summary_memory_state::regions_getter_t getter, ptr_set_t &region_aliases_c, region_t &region_so) {
  summary_memory_state *return_site = *(this->return_site);

  for(auto &field_mapping_s : region_so) {
    field &f_s = field_mapping_s.second;
    //      id_shared_t id_me = me_copy->transVar(region_key, field_mapping_s.first, f_s.size);

    //      cout << "    " << *id_me << endl;

    num_var *nv_s = new num_var(f_s.num_id);
    ptr_set_t aliases_s = summary->child_state->queryAls(nv_s);

    ptr_set_t aliases_c;
    ptr_set_t aliases_s_heap;
    for(auto &alias_s : aliases_s) {
      summy::rreil::id_visitor idv;
      bool _continue = false;
      idv._([&](allocation_memory_id const *) {
        aliases_s_heap.insert(alias_s);
        aliases_c.insert(alias_s);
        _continue = true;
      });
      idv._([&](sm_id const *) {
        aliases_c.insert(alias_s);
        _continue = true;
      });
      idv._([&](special_ptr const *sp) {
        if(sp->get_kind() == summy::rreil::NULL_PTR)
          aliases_c.insert(alias_s);
        else if(sp->get_kind() == summy::rreil::BAD_PTR)
          aliases_c.insert(ptr(special_ptr::badptr, vs_finite::top));
        else
          assert(false);
        _continue = true;
      });
      alias_s.id->accept(idv);
      if(_continue) continue;

      //          idv._default([&](id *_) {
      auto aliases_mapped_it = ptr_map.find(alias_s.id);
      ptr_set_t const &aliases_c_next = aliases_mapped_it->second;
      //        assert(aliases_mapped_it != alias_map.end() && aliases_me_ptr.size() > 0);
      //        cout << "search result for " << *alias_s.id << ": " << (aliases_mapped_it !=
      //        ptr_map.end()) << endl;
      if(aliases_mapped_it != ptr_map.end()) {
        for(auto alias_c_next : aliases_c_next) {
          //            cout << *alias_c_next.offset << " + " << *alias_s.offset << " = "
          //                 << *(*alias_c_next.offset + alias_s.offset) << endl;
          aliases_c.insert(ptr(alias_c_next.id, *alias_c_next.offset + alias_s.offset));
        }
      }
      //          });
      //          alias_s.id->accept(idv);
    }

    /*
     * If a heap pointer is re-introduced by the summary we have to be careful
     * since we actually get a summary memory region for the heap region now
     * in the caller state. We represent this by adding the bad pointer to the
     * respective alias set in the caller state. We have to add the bad
     * pointer if we find a heap pointer in the caller state and in an alias
     * set of the summary.
     *
     * This is because heap memory of the caller
     * is accessed through a register not known to point to the heap in the
     * callee. Thus, whenever we see a heap id in the caller and in the summary,
     * it must have propagated from the callee through the return site back to
     * the call.
     *
     * In the following lines, we calculate the set of heap ids that
     * we find in the output of the summary.
     */
    id_set_t aliases_so_heap_ids;
    for(auto &alias : aliases_s_heap)
      aliases_so_heap_ids.insert(alias.id);
    /*
     * We conclude that we need to add the bad pointer if we find one of the
     * aliases in the caller state.
     */
    id_set_t reintroduction_heap_ids;
    set_intersection(aliases_so_heap_ids.begin(), aliases_so_heap_ids.end(),
      caller_alloc_deref.begin(), caller_alloc_deref.end(),
      inserter(reintroduction_heap_ids, reintroduction_heap_ids.begin()), id_less());
    bool heap_reintroduction = reintroduction_heap_ids.size() > 0;

    vs_shared_t value_summary = summary->child_state->queryVal(nv_s);
    num_expr *value_summary_expr = new num_expr_lin(new num_linear_vs(value_summary));

    /*
     * The following function adds the bad pointer to an alias set if a
     * heap region is re-introduced in the summary.
     */
    auto add_heapbad = [&](api::num_var *nv_fld_c) {
      ptr_set_t aliases_c_assignment = aliases_c;
      if(heap_reintroduction)
        aliases_c_assignment.insert(ptr(special_ptr::badptr, vs_finite::zero));
      return aliases_c_assignment;
    };

    summary_memory_state::updater_t strong = [&](api::num_var *nv_fld_c) {
      //        cout << *nv_fld_c << " <- " << aliases_c << endl;
      ptr_set_t aliases_c_assignment = add_heapbad(nv_fld_c);
      return_site->child_state->kill({nv_fld_c});
      if(aliases_c.size() > 0) {
        return_site->child_state->assign(nv_fld_c, aliases_c_assignment);
      } else
        return_site->child_state->assign(nv_fld_c, value_summary_expr);
    };
    summary_memory_state::updater_t weak = [&](api::num_var *nv_fld_c) {
      //        cout << "weak for " << *nv_fld_c << endl;
      ptr_set_t aliases_c_assignment = add_heapbad(nv_fld_c);
      if(aliases_c_assignment.size() > 0) {
        ptr_set_t aliases_joined_c = return_site->child_state->queryAls(nv_fld_c);
        return_site->child_state->kill({nv_fld_c});
        aliases_joined_c.insert(aliases_c_assignment.begin(), aliases_c_assignment.end());
        return_site->child_state->assign(nv_fld_c, aliases_joined_c);
      } else {
        vs_shared_t value_caller = return_site->child_state->queryVal(nv_fld_c);
        return_site->child_state->kill({nv_fld_c});
        num_expr *value_joined_expr =
          new num_expr_lin(new num_linear_vs(value_set::join(value_summary, value_caller)));
        return_site->child_state->assign(nv_fld_c, value_joined_expr);
        delete value_joined_expr;
      }
    };
    ptr_set_t region_aliases_c_offset_bits =
      offsets_bytes_to_bits_base(field_mapping_s.first, region_aliases_c);

    //            cout << "~~~" << region_aliases_c_offset_bits << ":" << f_s.size << endl;
    return_site->update_multiple(
      region_aliases_c_offset_bits, getter, f_s.size, strong, weak, true, true);

    delete value_summary_expr;
    delete nv_s;
  }
};

summary_memory_state *analysis::summary_application::apply_summary() {
  assert(!return_site);

  return_site = caller->copy();
  summary_memory_state *return_site = *(this->return_site);

  if(summary->is_bottom()) {
    return_site->bottomify();
    return return_site;
  }
  //    cout << "apply_summary" << endl;
  //    cout << "caller:" << endl
  //         << *caller << endl;
  //  cout << "summary: " << endl
  //       << *summary << endl;

  //    caller->check_consistency();
  //    summary->check_consistency();

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

  /*
   * Todo: Handling of null pointer and bad pointer?
   */
  ptr_map.insert(
    make_pair(special_ptr::_nullptr, ptr_set_t{ptr(special_ptr::_nullptr, vs_finite::zero)}));
  ptr_map.insert(
    make_pair(special_ptr::badptr, ptr_set_t{ptr(special_ptr::badptr, vs_finite::zero)}));

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
   * Build the static and dynamically allocated memory mapping from the input.
   */
  for(auto &region_mapping_si : summary->input.deref) {
    id_shared_t region_key_summary = region_mapping_si.first;

    bool static_or_dynamic = false;
    summy::rreil::id_visitor idv;
    idv._([&](allocation_memory_id const *ami) { static_or_dynamic = true; });
    idv._([&](sm_id const *) { static_or_dynamic = true; });
    region_key_summary->accept(idv);
    if(!static_or_dynamic) continue;
    ptr_set_t region_keys_c = ptr_set_t({ptr(region_key_summary, vs_finite::zero)});
    ptr_map.insert(make_pair(region_key_summary, region_keys_c));
    build_pmap_region(region_key_summary, region_keys_c, &relation::get_deref);
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
   * Having built the relation between variable and pointers names of the caller state and the
   * summary,
   * we apply the summary by updating the caller using the summary state.
   */

  //  cout << "middle: " << *return_site << endl;

  /*
   * Application
   */

  /*
   * We build a ptr_map_rev and a conflict region map in order to deal with unexpected aliasing
   * situations
   */
  map<id_shared_t, ptr_set_t, id_less> ptr_map_rev;
  for(auto &ptr_mapping : ptr_map)
    for(auto &p_c : ptr_mapping.second)
      if(summary->output.deref.find(ptr_mapping.first) != summary->output.deref.end())
        ptr_map_rev[p_c.id].insert(ptr(ptr_mapping.first, p_c.offset));

  bool dirty = false;
  for(auto &rev_mapping : ptr_map_rev) {
    ptr_set_t &ptrs_s = rev_mapping.second;
    if(ptrs_s.size() > 1) {
      map<int64_t, size_t> dirty_bits;

      for(auto &ptr : ptrs_s) {
        id_shared_t ptr_id = ptr.id;
        vs_shared_t offset = ptr.offset;
        optional<int64_t> offset_int;
        value_set_visitor vsv(true);
        vsv._([&](vs_finite const *vsf) {
          if(vsf->is_singleton()) offset_int = vsf->min();
        });
        offset->accept(vsv);
        if(!offset_int) {
          dirty = true;
          break;
        }
        int64_t offset_int_bits = offset_int.value() * 8;

        region_t &r = summary->output.deref.at(ptr_id);
        for(auto field_mapping : r) {
          int64_t offset_f = field_mapping.first + offset_int_bits;

          if(dirty_bits.find(offset_f) != dirty_bits.end()) {
            dirty = true;
            goto _collect_end;
          }
          if(field_mapping.second.size != 0) dirty_bits[offset_f] = field_mapping.second.size;
        }
      }
    _collect_end:
      if(dirty) break;

      optional<int64_t> offset;
      for(auto dirty_mapping : dirty_bits) {
        //        cout << "next dirt bag: " << dirty_mapping.first << ", with size: " <<
        //        dirty_mapping.second << endl;

        if(offset) {
          int offset_next = dirty_mapping.first;
          if(offset.value() >= offset_next) {
            dirty = true;
            break;
          }
        }
        offset = dirty_mapping.first + dirty_mapping.second - 1;
      }

      //      for(auto &__ptr : ptrs_s)
      //        cout << *__ptr << ", ";
      //      cout << endl;
      /*
       * Handling of unexpected aliases...
       */
      //      auto ptrs_it = ptrs_s.begin();
      //      region_t cr = summary->output.deref.at(*ptrs_it);
      //      while(++ptrs_it != ptrs_s.end())
      //        cr = join_region_aliases(cr, summary->output.deref.at(*ptrs_it),
      //        summary->child_state);
      //      conflict_regions[rev_mapping.first] = cr;
    }
  }
  if(dirty) {
    //    cout << "dirty :-(." << endl;
    return_site->topify();
    return return_site;
  }

  //  cout << "return_site, before app: " << endl
  //       << *return_site << endl;

  for(auto &deref_mapping_c : caller->input.deref) {
    summy::rreil::id_visitor idv;
    idv._([&](allocation_memory_id const *) { caller_alloc_deref.insert(deref_mapping_c.first); });
    deref_mapping_c.first->accept(idv);
  }

  num_expr *_top = new num_expr_lin(new num_linear_vs(value_set::top));


  for(auto &region_mapping_so : summary->output.regions) {
    id_shared_t region_key = region_mapping_so.first;
    //    cout << "REGION " << *region_key << endl;
    // region_t &region_si = summary->input.regions.at(region_key);

    ptr_set_t region_aliases_c = ptr_set_t({ptr(region_key, vs_finite::zero)});
    process_region(&relation::get_regions, region_aliases_c, region_mapping_so.second);
  }

  for(auto &deref_mapping_so : summary->output.deref) {
    id_shared_t region_key = deref_mapping_so.first;
    // region_t &region_si = summary->input.deref.at(region_key);

    auto region_aliases_c_it = ptr_map.find(region_key);

    /*
     * If a memory region is not reachable in the summary (?),
     * its key may not be present in the ptr_map
     */
    if(region_aliases_c_it == ptr_map.end()) continue;
    ptr_set_t &region_aliases_c = region_aliases_c_it->second;

    //    cout << "process_region(): " << region_aliases_c << endl;

    process_region(&relation::get_deref, region_aliases_c, deref_mapping_so.second);
  }
  delete _top;

  //  cout << "return_site: " << endl
  //       << *return_site << endl;

  num_vars *_vars = return_site->vars_relations();
  return_site->project(_vars);
  delete _vars;

  return return_site;
}
