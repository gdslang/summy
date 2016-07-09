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
#include <summy/analysis/domains/ptr_set.h>
#include <summy/analysis/domains/cr_merge_region_iterator.h>
#include <summy/analysis/domains/merge_region_iterator.h>
#include <summy/analysis/domains/numeric/als_state.h>
#include <summy/analysis/domains/summary_memory_state.h>
#include <summy/analysis/domains/util.h>
#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/memory_id.h>
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
using summy::rreil::allocation_memory_id;
using summy::rreil::ptr_memory_id;
using summy::rreil::sm_id;
using summy::rreil::special_ptr;
using summy::rreil::special_ptr_kind;

using namespace std;
using namespace analysis;
using namespace analysis::api;
using namespace summy;

std::tuple<bool, num_var_pairs_t, id_set_t>(::analysis::compatMatchSeparate)(bool widening, relation &a_in,
  relation &a_out, numeric_state *a_n, relation &b_in, relation &b_out, numeric_state *b_n) {
  /*
   * No more copy/paste; later, we can remove this entirely
   */
  bool copy_paste = false;
  bool conflicts = false;

  id_set_t merged_region_keys;

  /*
   * Find aliases for a specific region of the given summaries. We need both
   * input and output here in order to be able to add pointers that are missing
   * in one of the regions.
   */
  auto compatMatchSeparateRegion = [&](io_region &io_ra, io_region &io_rb) {
    //    cout << "Next region" << endl;

    num_var_pairs_t upcoming;

    //    cout << "io_ra: ";
    //    for(auto &f_it : io_ra.out_r)

    vector<function<void()>> insertions;
    vector<function<void()>> conflict_resolvers;

    /*
     * We first check for missing pointers in the region...
     */
    struct collision_t {
      int64_t from;
      int64_t to;
    };

    optional<collision_t> collision;

    auto resolve_collision = [&]() {
      if(collision) {
        conflicts = true;
        //        cout << "Adding conflict resolver for collision from " << collision->from << " to " << collision->to
        //        << endl;
        conflict_resolvers.push_back([&io_ra, &a_n, &io_rb, &b_n, collision]() {
          auto collision_v = collision.value();
          //          cout << "Resolving collision from " << collision_v.from << " to " << collision_v.to << endl;
          //                    summary_memory_state *a_sms_before = new summary_memory_state(NULL, a_n, a_in, a_out);
          //                    summary_memory_state *b_sms_before = new summary_memory_state(NULL, b_n, b_in, b_out);
          //                    cout << "a: " << *a_sms_before << endl;
          //                    cout << "b: " << *b_sms_before << endl;

          //          cout << "Resolving in a..." << endl;
          io_ra.retrieve_field(a_n, collision_v.from, collision_v.to - collision_v.from + 1, true, true);
          //          cout << "Resolving in b..." << endl;
          io_rb.retrieve_field(b_n, collision_v.from, collision_v.to - collision_v.from + 1, true, true);

          //          summary_memory_state *a_sms_after = new summary_memory_state(NULL, a_n, a_in, a_out);
          //          summary_memory_state *b_sms_after = new summary_memory_state(NULL, b_n, b_in, b_out);
          //          cout << "a: " << *a_sms_after << endl;
          //          cout << "b: " << *b_sms_after << endl;
        });
        collision = std::experimental::nullopt;
      }
    };

    //    cout << "a before:";
    //    for(auto r_it : io_ra.in_r)
    //      cout << "(@" << r_it.first << ":" << r_it.second.size << "#" << *r_it.second.num_id << ")";
    //    cout << endl;
    //    cout << "b before:";
    //    for(auto r_it : io_rb.in_r)
    //      cout << "(@" << r_it.first << ":" << r_it.second.size << "#" << *r_it.second.num_id << ")";
    //    cout << endl;

    merge_region_iterator mri(io_ra.in_r, io_rb.in_r);
    while(mri != merge_region_iterator::end(io_ra.in_r, io_rb.in_r)) {
      region_pair_desc_t rpd = *mri;

      //      cout << "Next it, collision: " << rpd.collision << endl;
      //      if(rpd.field_first_region())
      //        cout << "First region field: " << *rpd.field_first_region()->f.num_id
      //             << ", offset: " << rpd.field_first_region()->offset << ", size: " <<
      //             rpd.field_first_region()->f.size
      //             << endl;
      //      if(rpd.field_second_region())
      //        cout << "Second region field: " << *rpd.field_second_region()->f.num_id
      //             << ", offset: " << rpd.field_second_region()->offset << ", size: " <<
      //             rpd.field_second_region()->f.size
      //             << endl;

      if(rpd.collision) {
        if(rpd.ending_last) {
          //          cout << "Collision (first) from "
          //               << "from_current"
          //               << " s:" << rpd.ending_first.f.size << endl;
          //          cout << "Collision (second) from " << rpd.ending_last.value().offset
          //               << " s:" << rpd.ending_last.value().f.size << endl;
          int64_t from_current = std::min(rpd.ending_first.offset, rpd.ending_last->offset);
          int64_t from = collision ? collision->from : from_current;
          size_t size = rpd.ending_first.offset + rpd.ending_first.f.size - from;
          if(rpd.ending_last) size = std::max(size, rpd.ending_last->offset + rpd.ending_last->f.size - from);
          assert((int64_t)size > 0);
          collision = collision_t{from, from + (int64_t)size - 1};
        }
      } else {
        resolve_collision();

        if(!rpd.ending_last) {
          //          cout << "fr: " << *rpd.ending_first.f.num_id << " at " << rpd.ending_first.offset << endl;
          field_desc_t ending_first = rpd.ending_first;

          auto add_insertions = [&](numeric_state *x_n, numeric_state *y_n, io_region *io_rx, io_region *io_ry) {
            num_var ef_var(rpd.ending_first.f.num_id);
            ptr_set_t aliases = x_n->queryAls(&ef_var);
            bool has_no_badnull = false;
            for(auto &alias : aliases) {
              bool is_no_badnull = true;
              summy::rreil::id_visitor idv;
              idv._([&](special_ptr *p) { is_no_badnull = false; });
              alias.id->accept(idv);
              if(is_no_badnull) has_no_badnull = true;
            }

            insertions.push_back(
              [copy_paste, has_no_badnull, io_rx, io_ry, x_n, y_n, ending_first /*, &copy_pasters*/]() {
                field inserted;
                if(copy_paste || has_no_badnull)
                  inserted = io_ry->retrieve_field(y_n, ending_first.offset, ending_first.f.size, false, true).f.value();
                else
                  inserted =
                    io_ry->retrieve_field(y_n, ending_first.offset, ending_first.f.size, false, true, [](auto...) {
                      ptr _badptr = ptr(special_ptr::badptr, vs_finite::zero);
                      return ptr_set_t({_badptr});
                    }).f.value();

                if(copy_paste) {
                  num_var *from = new num_var(io_rx->out_r.at(ending_first.offset).num_id);
                  num_var *to = new num_var(inserted.num_id);
                  y_n->copy_paste(to, from, x_n);
                  delete to;
                  delete from;
                }

              });
          };

          if(ending_first.region_first)
            add_insertions(a_n, b_n, &io_ra, &io_rb);
          else
            add_insertions(b_n, a_n, &io_rb, &io_ra);
        }
      }
      ++mri;
    }
    resolve_collision();

    /*
     * ... insert them...
     */
    for(auto inserter : insertions)
      inserter();

    for(auto conflict_resolver : conflict_resolvers)
      conflict_resolver();

    //    cout << "a:";
    //    for(auto r_it : io_ra.in_r)
    //      cout << "(@" << r_it.first << ":" << r_it.second.size << "#" << *r_it.second.num_id << ")";
    //    cout << endl;
    //    cout << "b:";
    //    for(auto r_it : io_rb.in_r)
    //      cout << "(@" << r_it.first << ":" << r_it.second.size << "#" << *r_it.second.num_id << ")";
    //    cout << endl;

    /*
     * ... and finally retrieve all matching pointer variables. Keep in mind
     * that there is always at most one alias per numeric variable in
     * the input.
     */
    mri = merge_region_iterator(io_ra.in_r, io_rb.in_r);
    while(mri != merge_region_iterator::end(io_ra.in_r, io_rb.in_r)) {
      region_pair_desc_t rpd = *mri;

      //            cout << "Var first: " << *rpd.ending_first.f.num_id << endl;
      //      cout << "Offset/size first: " << rpd.ending_first.offset << " / " << rpd.ending_first.f.size << endl;
      //      if(rpd.ending_last)
      //        cout << "Offset/size last: " << rpd.ending_last.value().offset << " / " <<
      //        rpd.ending_last.value().f.size
      //             << endl;
      //      cout << "collision: " << rpd.collision << endl;

      if(!rpd.collision) {
        if(rpd.ending_last) {
          field const &f_a = rpd.ending_first.f;
          field const &f_b = rpd.ending_last.value().f;
          if(f_a.size == f_b.size) {
            num_var *f_a_nv = new num_var(f_a.num_id);
            ptr_set_t als_a = a_n->queryAls(f_a_nv);
//                        cout << "************" << *f_a_nv << ": " << als_a << endl;
            delete f_a_nv;

            ptr p_a = unpack_singleton(als_a);

            num_var *f_b_nv = new num_var(f_b.num_id);
            ptr_set_t als_b = b_n->queryAls(f_b_nv);
            delete f_b_nv;
            //            cout << "together with: " << als_b << endl;
            ptr p_b = unpack_singleton(als_b);

            //            assert((*p_a.id == *special_ptr::badptr) == (*p_b.id == *special_ptr::badptr));

            /*
             * Todo: Check for badptr?
             */
            if(!(*p_a.id == *special_ptr::badptr) && !((*p_b.id == *special_ptr::badptr))) {
              //              cout << "pushing aliases... " << p_a << ", " << p_b << endl;

              //                            cout << p_a << endl;
              assert(*p_a.offset == vs_finite::zero);
              assert(*p_b.offset == vs_finite::zero);

              upcoming.push_back(make_tuple(new num_var(p_a.id), new num_var(p_b.id)));
            }
          }
        } else {
          /*
           * We should not get here since we add partner fields above.
           */
          assert(false);
        }
      }
      ++mri;
    }

    //    for(auto copy_paster : copy_pasters)
    //      copy_paster();

    return upcoming;
  };

  struct region_pair {
    io_region io_ra;
    io_region io_rb;

    //    region_pair(region_pair &o) : io_ra(o.io_ra), io_rb(o.io_rb) {
    //    }
  };
  queue<region_pair> worklist;
  auto wl_push = [&](id_shared_t key, region_pair rp) {
    merged_region_keys.insert(key);
    worklist.push(rp);
  };

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
          //          if(!a_b && widening) continue;
          tie(regions_second_it, ignore) = second_in.insert(make_pair(regions_first_it->first, region_t()));
          second_out.insert(make_pair(regions_first_it->first, region_t()));
        } else if(a_b)
          continue;

        auto &regions_first_out = first_out.at(regions_first_it->first);
        auto &regions_second_out = second_out.at(regions_first_it->first);
        io_region io_first = io_region(regions_first_it->second, regions_first_out, regions_first_it->first);
        io_region io_second = io_region(regions_second_it->second, regions_second_out, regions_second_it->first);
        if(a_b)
          wl_push(regions_first_it->first, region_pair{io_first, io_second});
        else
          wl_push(regions_second_it->first, region_pair{io_second, io_first});
      }
    };
  init_from_regions(a_in.regions, a_out.regions, b_in.regions, b_out.regions, true);
  init_from_regions(b_in.regions, b_out.regions, a_in.regions, a_out.regions, false);

  auto init_from_deref_no_ptr =
    [&](region_map_t &first_in, region_map_t &first_out, region_map_t &second_in, region_map_t &second_out, bool a_b) {
      for(auto regions_first_it = first_in.begin(); regions_first_it != first_in.end(); regions_first_it++) {

        /*
         * We don't identify ptr memory regions by name. We only add ptr memory regions
         * if found during memory matching. If there is matching ptr memory region in the
         * state, this may be a dangling, older version of the existing memory region. We could
         * do one of the following:
         *
         * - Do we want to do a three-way-join of the two non-dangling memory regions (one of them
         * may have been introduced freshly) and the dangling region?
         * - We need to decide on whether to keep dangling regions or not (they may still be referenced in the
         * output). Right now they seem to be discarded.
         * - Especially if we decide to keep dangling regions, we need to get rid of them sometime,
         * possibly during widening.
         */
        bool is_ptr_var = false;
        summy::rreil::id_visitor idv;
        idv._([&](ptr_memory_id *pmid) { is_ptr_var = true; });
        regions_first_it->first->accept(idv);
        if(is_ptr_var) continue;

        auto regions_second_it = second_in.find(regions_first_it->first);
        if(regions_second_it == second_in.end()) {
          /*
           * Deref regions referenced by pointers are only added if matched in the input. Therefore,
           * here we only add if the region key is not a ptr_memory_id.
           */

          tie(regions_second_it, ignore) = second_in.insert(make_pair(regions_first_it->first, region_t()));
          second_out.insert(make_pair(regions_first_it->first, region_t()));
        } else
          /*
           * There may be special regions that are not reachable through registers, but have globally unique
           * ids. Therefore, we add deref regions with equal ids to the worklist. If a_b is true, the region
           * pair has already been handled when a_b was false.
           */
          if(a_b)
          continue;


        auto &regions_first_out = first_out.at(regions_first_it->first);
        auto &regions_second_out = second_out.at(regions_first_it->first);
        io_region io_first = io_region(regions_first_it->second, regions_first_out);
        io_region io_second = io_region(regions_second_it->second, regions_second_out);
        if(a_b)
          wl_push(regions_first_it->first, region_pair{io_first, io_second});
        else
          wl_push(regions_second_it->first, region_pair{io_second, io_first});
      }
    };
  init_from_deref_no_ptr(a_in.deref, a_out.deref, b_in.deref, b_out.deref, true);
  init_from_deref_no_ptr(b_in.deref, b_out.deref, a_in.deref, a_out.deref, false);

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

    num_var_pairs_t upcoming = compatMatchSeparateRegion(rp.io_ra, rp.io_rb);
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

      if(widening && deref_a_in_it == a_in.deref.end()) {
        /*
         * During widening, we don't propagate nameless regions,
         * since this can result in generating more and more regions
         */
        bool ptr_mem_region = false;
        summy::rreil::id_visitor idv;
        idv._([&](ptr_memory_id *idv) { ptr_mem_region = true; });
        vb->get_id()->accept(idv);
        if(ptr_mem_region) {
          delete va;
          delete vb;
          continue;
        }
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
      io_region io_a = io_region(deref_a_in_it->second, deref_a_out);
      io_region io_b = io_region(deref_b_in_it->second, deref_b_out);
      //      cout << "Pushing pair for " << *deref_a_in_it->first << "/" << *deref_b_in_it->first << endl;
      //      cout << "Current queue size: " << worklist.size() << endl;
      //      cout << "io_b.in_r.size(): " << io_b.in_r.size() << endl;
      wl_push(deref_a_in_it->first, region_pair{io_a, io_b});

      delete va;
      delete vb;
    }
  }

  return make_tuple(conflicts, result, merged_region_keys);
}

std::tuple<bool, memory_head, numeric_state *, numeric_state *>(::analysis::compat)(
  bool widening, const summary_memory_state *a, const summary_memory_state *b) {
  numeric_state *a_n = a->child_state->copy();
  numeric_state *b_n = b->child_state->copy();


  struct field_converage {
    int64_t from;
    size_t size;

    field_converage(region_t const &r) {
      auto it = r.begin();
      if(it == r.end()) {
        from = size = 0;
        return;
      }
      from = it->first;
      //      bits = it->second.size;
      int64_t end = from + it->second.size - 1;
      it++;
      while(it != r.end()) {
        end = it->first + it->second.size - 1;
        //        bits += it->second.size;
        it++;
      }
      size = end - from + 1;
    }

    field_converage(field_converage const &o) : from(o.from), size(o.size) {}

    field_converage(int64_t from, size_t size) : from(from), size(size) {}

    bool operator==(field_converage &o) {
      return from == o.from && size == o.size;
    }

    static field_converage max(field_converage x, field_converage y) {
      return field_converage(std::min(x.from, y.from), std::max(x.size, y.size));
    }
  };
  map<id_shared_t, field_converage, id_less_no_version> field_counts;
  for(auto r_it : a->input.regions) {
    //    cout << "Adding " << *r_it.first << endl;
    field_counts.insert(pair<id_shared_t, field_converage>(r_it.first, field_converage(r_it.second)));
  }
  for(auto r_it : b->input.regions) {
    //    cout << "Adding2 " << *r_it.first << endl;
    field_converage fc_new = field_converage(field_converage(r_it.second));
    auto fc_it = field_counts.find(r_it.first);
    if(fc_it == field_counts.end())
      field_counts.insert(pair<id_shared_t, field_converage>(r_it.first, fc_new));
    else
      fc_it->second = field_converage::max(fc_it->second, fc_new);
  }

  if(a->is_bottom()) {
    memory_head head;
    head.input = b->input;
    head.output = b->output;
    return make_tuple(false, head, a_n, b_n);
  } else if(b->is_bottom()) {
    memory_head head;
    head.input = a->input;
    head.output = a->output;
    return make_tuple(false, head, a_n, b_n);
  }

  //    ((summary_memory_state *)a)->check_consistency();
  //    ((summary_memory_state *)b)->check_consistency();

  //  static int c = 0;
  //  printf("%d\n", c++);

  //  if(!a_n->is_bottom() && !b_n->is_bottom()) {
  //  cout << "++++++++++++++++++++++++++++++" << endl;
  //  cout << "++++++++++++++++++++++++++++++" << endl;
  //  cout << "++++++++++++++++++++++++++++++" << endl;
  //  if(c == 1688) {
  //  cout << "compat OF" << endl;
  //  cout << *a << endl;
  //  cout << "WITH" << endl;
  //  cout << *b << endl;
  //  }
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
    //    assert(rel.deref.find(to) == rel.deref.end());
    auto rel_it = rel.deref.find(from);
    if(rel_it != rel.deref.end()) {
      if(rel.deref.find(to) != rel.deref.end()) {
        /*
         * Todo: Dangling regions are not removed properly. This way, the can be
         * propagated back to somewhere they used to exist. This needs to
         * be fixed!
         */
        cout << "Bug todo: Rename target already exists!" << endl;
        rel.deref.erase(to);
      }

      region_t region = rel_it->second;
      rel.deref.erase(rel_it);
      rel.deref.insert(make_pair(to, region));
    }
  };

  /*
   * The first step is implemented by finding corresponding pointer variables...
   */
  num_var_pairs_t eq_aliases;
  id_set_t merged_region_keys;
  bool conflicts;
  tie(conflicts, eq_aliases, merged_region_keys) =
    compatMatchSeparate(widening, a_input, a_output, a_n, b_input, b_output, b_n);

  //  summary_memory_state *before_rename = new summary_memory_state(a->sm, b_n, b_input, b_output);
  //  cout << "before_rename: " << *before_rename << endl;

  /*
   * ... and equating them in the respective numeric state. Additionally, the keys of the memory regions
   * in the deref map need to be replaced.
   */
  b_n->equate_kill(eq_aliases);
  //  a_n->equate_kill(eq_aliases);

  //  summary_memory_state *x = new summary_memory_state(a->sm, false, a_n, a_input, a_output);
  //  summary_memory_state *y = new summary_memory_state(b->sm, false, b_n, b_input, b_output);
  //  cout << *x << endl;
  //  cout << "GRRRRRRR" << endl;
  //  cout << *y << endl;

  for(auto &eq_alias : eq_aliases) {
    num_var *alias_a;
    num_var *alias_b;
    tie(alias_a, alias_b) = eq_alias;
    //    cout << "eq_alias: " << *alias_a << " <- " << *alias_b << endl;
    rename_rk(b_input, alias_b->get_id(), alias_a->get_id());
    rename_rk(b_output, alias_b->get_id(), alias_a->get_id());
    delete alias_a;
    delete alias_b;
  }

  //    summary_memory_state *after_rename_a = new summary_memory_state(a->sm, a_n, a_input, a_output);
  //    cout << "after_rename, a: " << *after_rename_a << endl;
  //    summary_memory_state *after_rename_b = new summary_memory_state(a->sm, b_n, b_input, b_output);
  //    cout << "after_rename, b: " << *after_rename_b << endl;

  /*
   * In the second step, all corresponding regions already have got the same region key. Thus,
   * in order to build a compatible relation we only need to iterate the regions in the deref and
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
      region_t region;
      cr_merge_region_iterator mri(region_a, region_b, region);
      while(mri != cr_merge_region_iterator::end(region_a, region_b)) {
        region_pair_desc_t rpd = *mri;
        if(rpd.collision) {
          /*
           * Conflicts are now resolved during pointer machting, so there must
           * not be any conflicts left
           */
          cout << *id << endl;
          assert(false);
        } else {
          if(!rpd.ending_last) {
            cout << *id << endl;
            cout << *rpd.ending_first.f.num_id << endl;
          }
          assert(rpd.ending_last);
          //          if(rpd.ending_last) {
          field_desc_t fd_first = rpd.field_first_region().value();
          field_desc_t fd_second = rpd.field_second_region().value();

          id_shared_t id_first = fd_first.f.num_id;
          id_shared_t id_second = fd_second.f.num_id;
          if(!(*id_first == *id_second))
            equate_kill_vars.push_back(make_tuple(new num_var(id_first), new num_var(id_second)));
          region.insert(make_pair(fd_first.offset, fd_first.f));
          //          }
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

      result_map.insert(make_pair(id, region));
    };
    for(auto &region_it : a_map) {
      if(merged_region_keys.find(region_it.first) == merged_region_keys.end()) continue;
      auto region_b_it = b_map.find(region_it.first);
      assert(region_b_it != b_map.end());
      handle_region(region_it.first, region_it.second, region_b_it->second);
      //      else {
      //        bool insert_other = true;
      //        summy::rreil::id_visitor idv;
      //        idv._([&](ptr_memory_id *pmid) {
      //          insert_other = false;
      //        });
      //        if(insert_other)
      //          handle_region(region_it.first, region_it.second, region_t());
      //      }
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


  assert(head.input.regions.size() >= max(a->input.regions.size(), b->input.regions.size()));
  for(auto r_it : head.input.regions) {
    auto fc_it = field_counts.find(r_it.first);
    if(fc_it == field_counts.end()) cout << *r_it.first << endl;
    assert(fc_it != field_counts.end());
    assert(field_converage(r_it.second) == fc_it->second);
  }

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
  return make_tuple(conflicts, head, a_n, b_n);
}
