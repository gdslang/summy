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
#include <summy/analysis/domains/cr_merge_region_iterator.h>
#include <summy/analysis/domains/merge_region_iterator.h>
#include <summy/analysis/domains/summary_memory_state.h>
#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/sm_id.h>
#include <summy/rreil/id/special_ptr.h>
#include <summy/value_set/value_set.h>
#include <summy/value_set/vs_finite.h>
#include <cppgdsl/rreil/rreil.h>
#include <map>
#include <queue>
#include <algorithm>
#include <experimental/optional>
#include <utility>

using gdsl::rreil::id;
using std::experimental::optional;
using summy::rreil::sm_id;
using summy::rreil::special_ptr;

using namespace std;
using namespace analysis;
using namespace analysis::api;
using namespace summy;

summary_memory_state * ::analysis::apply_summary(summary_memory_state *caller, summary_memory_state *summary) {
  summary_memory_state *return_site = caller->copy();

  //    cout << "apply_summary" << endl;
  //    cout << "caller:" << endl
  //         << *caller << endl;
  //    cout << "summary: " << endl
  //         << *summary << endl;

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

  /*
   * Todo: Handling of null pointer and bad pointer?
   */
  ptr_map.insert(make_pair(special_ptr::_nullptr, ptr_set_t{ptr(special_ptr::_nullptr, vs_finite::zero)}));
  ptr_map.insert(make_pair(special_ptr::badptr, ptr_set_t{ptr(special_ptr::badptr, vs_finite::zero)}));

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

  //  cout << "middle: " << *return_site << endl;

  /*
   * Application
   */

  /*
   * We build a ptr_map_rev and a conflict region map in order to deal with unexpected aliasing situations
   */
  map<id_shared_t, ptr_set_t, id_less_no_version> ptr_map_rev;
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
        value_set_visitor vsv;
        vsv._([&](vs_finite *vsf) {
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
        //        cout << "next dirt bag: " << dirty_mapping.first << ", with size: " << dirty_mapping.second << endl;

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
      //        cr = join_region_aliases(cr, summary->output.deref.at(*ptrs_it), summary->child_state);
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

  num_expr *_top = new num_expr_lin(new num_linear_vs(value_set::top));
  auto process_region =
    [&](summary_memory_state::regions_getter_t getter, ptr_set_t &region_aliases_c, region_t &region) {
      for(auto &field_mapping_s : region) {
        field &f_s = field_mapping_s.second;
        //      id_shared_t id_me = me_copy->transVar(region_key, field_mapping_s.first, f_s.size);

        //      cout << "    " << *id_me << endl;

        num_var *nv_s = new num_var(f_s.num_id);
        ptr_set_t aliases_s = summary->child_state->queryAls(nv_s);

        ptr_set_t aliases_c;
        for(auto &alias_s : aliases_s) {
          //          summy::rreil::id_visitor idv;
          //          idv._([&](sm_id *_sm_id) { aliases_c.insert(alias_s); });
          //          idv._default([&](id *_) {
          auto aliases_mapped_it = ptr_map.find(alias_s.id);
          ptr_set_t const &aliases_c_next = aliases_mapped_it->second;
          //        assert(aliases_mapped_it != alias_map.end() && aliases_me_ptr.size() > 0);
          //        cout << "search result for " << *_ptr.id << ": " << (aliases_mapped_it != alias_map.end()) <<
          //        endl;
          if(aliases_mapped_it != ptr_map.end())
            for(auto alias_c_next : aliases_c_next)
              aliases_c.insert(ptr(alias_c_next.id, *alias_c_next.offset + alias_s.offset));
          //          });
          //          alias_s.id->accept(idv);
        }

        vs_shared_t value_summary = summary->child_state->queryVal(nv_s);
        num_expr *value_summary_expr = new num_expr_lin(new num_linear_vs(value_summary));

        summary_memory_state::updater_t strong = [&](api::num_var *nv_fld_c) {
          //        cout << "strong for " << *nv_me << ": " << aliases_me << endl;
          return_site->child_state->kill({nv_fld_c});
          if(aliases_c.size() > 0)
            return_site->child_state->assign(nv_fld_c, aliases_c);
          else
            return_site->child_state->assign(nv_fld_c, value_summary_expr);
        };
        summary_memory_state::updater_t weak = [&](api::num_var *nv_fld_c) {
          //        cout << "weak for " << *nv_me << ": " << aliases_me << endl;
          if(aliases_c.size() > 0) {
            ptr_set_t aliases_joined_c = return_site->child_state->queryAls(nv_fld_c);
            return_site->child_state->kill({nv_fld_c});
            aliases_joined_c.insert(aliases_c.begin(), aliases_c.end());
            return_site->child_state->assign(nv_fld_c, aliases_joined_c);
          } else {
            vs_shared_t value_caller = return_site->child_state->queryVal(nv_fld_c);
            return_site->child_state->kill({nv_fld_c});
            num_expr *value_joined_expr =
              new num_expr_lin(new num_linear_vs(value_set::join(value_summary, value_caller)));
            return_site->child_state->assign(nv_fld_c, value_joined_expr);
          }
        };
        ptr_set_t region_aliases_c_offset_bits = offsets_bytes_to_bits_base(field_mapping_s.first, region_aliases_c);

        //      cout << "~~~" << region_aliases_c_offset_bits << ":" << f_s.size << endl;
        return_site->update_multiple(region_aliases_c_offset_bits, getter, f_s.size, strong, weak, true, true);

        delete value_summary_expr;
        delete nv_s;
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

  //  cout << "return_site: " << endl
  //       << *return_site << endl;

  return return_site;
}

num_var_pairs_t(::analysis::matchPointers)(
  relation &a_in, relation &a_out, numeric_state *a_n, relation &b_in, relation &b_out, numeric_state *b_n) {
  /*
   * Find aliases for a specific region of the given summaries. We need both
   * input and output here in order to be able to add pointers that are missing
   * in one of the regions.
   */
  auto matchPointersRegion = [&](io_region &io_ra, io_region &io_rb) {
    num_var_pairs_t upcoming;

    vector<function<void()>> insertions;

    /*
     * We first check for missing pointers in the region...
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
              //              cout << "Insertion of " << *ending_first.f.num_id << " into io_rb at " <<
              //              ending_first.offset << endl;
              io_rb.insert(b_n, ending_first.offset, ending_first.f.size, false);
            });
          else
            insertions.push_back([&io_ra, &a_n, ending_first]() {
              //              cout << "Insertion of " << *ending_first.f.num_id << " into io_ra at " <<
              //              ending_first.offset << endl;
              io_ra.insert(a_n, ending_first.offset, ending_first.f.size, false);
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
            //            cout << *f_a_nv << endl;
            ptr_set_t als_a = a_n->queryAls(f_a_nv);
            //            cout << als_a << endl;
            assert(als_a.size() == 1);
            delete f_a_nv;

            num_var *f_b_nv = new num_var(f_b.num_id);
            ptr_set_t als_b = b_n->queryAls(f_b_nv);
            assert(als_b.size() == 1);
            delete f_b_nv;

            ptr p_a = *als_a.begin();
            ptr p_b = *als_b.begin();
            if(!(*p_a.id == *special_ptr::badptr) && !((*p_b.id == *special_ptr::badptr))) {
              //              cout << "pushing aliases... " << endl;

              //              cout << p_a << endl;
              assert(*p_a.offset == vs_finite::zero);
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
  auto init_from_regions =
    [&](region_map_t &first_in, region_map_t &first_out, region_map_t &second_in, region_map_t &second_out, bool a_b) {
      for(auto regions_first_it = first_in.begin(); regions_first_it != first_in.end(); regions_first_it++) {
        auto regions_second_it = second_in.find(regions_first_it->first);
        if(regions_second_it == second_in.end()) {
          tie(regions_second_it, ignore) = second_in.insert(make_pair(regions_first_it->first, region_t()));
          second_out.insert(make_pair(regions_first_it->first, region_t()));
        } else if(a_b)
          continue;

        auto &regions_first_out = first_out.at(regions_first_it->first);
        auto &regions_second_out = second_out.at(regions_first_it->first);
        io_region io_first = io_region{regions_first_it->second, regions_first_out};
        io_region io_second = io_region{regions_second_it->second, regions_second_out};
        if(a_b)
          worklist.push(region_pair{io_first, io_second});
        else
          worklist.push(region_pair{io_second, io_first});
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
      if(!(*a->get_id() == *b->get_id())) result.push_back(make_tuple(a->copy(), b->copy()));
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
       * p_1 which need to be copied to the other i/o relation. Therefore, we add the
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
      io_region io_a = io_region{deref_a_in_it->second, deref_a_out};
      io_region io_b = io_region{deref_b_in_it->second, deref_b_out};
      worklist.push(region_pair{io_a, io_b});

      delete va;
      delete vb;
    }
  }

  return result;
}

std::tuple<memory_head, numeric_state *, numeric_state *>(::analysis::compat)(
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
  //  cout << "++++++++++++++++++++++++++++++" << endl;
  //  cout << "++++++++++++++++++++++++++++++" << endl;
  //  cout << "++++++++++++++++++++++++++++++" << endl;
  //  cout << "compat OF" << endl;
  //  cout << *a << endl;
  //  cout << "WITH" << endl;
  //  cout << *b << endl;
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

  //  summary_memory_state *after_rename_a = new summary_memory_state(a->sm, a_n, a_input, a_output);
  //  cout << "after_rename, a: " << *after_rename_a << endl;
  //  summary_memory_state *after_rename_b = new summary_memory_state(a->sm, b_n, b_input, b_output);
  //  cout << "after_rename, b: " << *after_rename_b << endl;

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
        if(!(*x == *y)) equate_kill_vars.push_back(make_tuple(new num_var(x), new num_var(y)));
      };
      id_set_t a_kill_ids;
      id_set_t b_kill_ids;

      region_t region;

      cr_merge_region_iterator mri(region_a, region_b, region);
      while(mri != cr_merge_region_iterator::end(region_a, region_b)) {
        region_pair_desc_t rpd = *mri;
        if(rpd.collision) {
          if(rpd.ending_last) {
            a_kill_ids.insert(rpd.field_first_region().value().f.num_id);
            b_kill_ids.insert(rpd.field_second_region().value().f.num_id);
          }
        } else {
          if(!rpd.ending_last) {
            //            cout << *id << endl;
            //            cout << *rpd.ending_first.f.num_id << endl;
          }
          assert(rpd.ending_last);

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
      if(region_b_it != b_map.end()) handle_region(region_it.first, region_it.second, region_b_it->second);
      //      else
      //        handle_region(region_it.first, region_it.second, region_t());
    }
    //    for(auto &region_b_it : b_map) {
    //      if(a_map.find(region_b_it.first) == a_map.end()) handle_region(region_b_it.first, region_t(),
    //      region_b_it.second);
    //    }
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
