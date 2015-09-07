/*
 * als_state.cpp
 *
 *  Created on: Mar 20, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/numeric/als_state.h>
#include <summy/analysis/domains/api/api.h>
#include <summy/rreil/id/numeric_id.h>
#include <summy/value_set/value_set.h>
#include <algorithm>
#include <assert.h>
#include <cppgdsl/rreil/rreil.h>
#include <summy/rreil/id/id_visitor.h>
#include <summy/rreil/id/sm_id.h>
#include <summy/rreil/id/special_ptr.h>
#include <experimental/optional>

using gdsl::rreil::id;
using std::experimental::optional;

using namespace analysis;
using namespace analysis::api;
using namespace std;
using namespace summy;
using namespace summy::rreil;

/*
 * Saubere Implementierung des Alias-Domain sollte eine spezielle, neue Variable
 * für den Offset verwenden und nicht diejenige, die schon für den Wert der Pointervariablen
 * verwendet wird.
 *
 * => So wie es aktuell ist, ist die Alias-Domain instransparent, bei Assumptions auf
 * Pointern muss man die Aliasing-Beziehung verlieren oder auf die Einschränkung der
 * Variablen verzichten...
 */

void analysis::als_state::kill(id_shared_t id) {
  num_var *id_var = new num_var(id);
  auto id_it = elements.find(id);
  if(id_it != elements.end()) {
    id_shared_t id_repl;
    id_set_t &aliases = id_it->second;
    if(aliases.size() == 1)
      id_repl = *aliases.begin();
    else
      id_repl = numeric_id::generate();
    num_expr *kill_expr = new num_expr_lin(new num_linear_term(new num_var(id_repl)));
    //      cout << "Assigning " << *current_val_expr << " to " << *var << endl;
    child_state->assign(id_var, kill_expr);
    delete kill_expr;
    elements.erase(id_it);
  }
  delete id_var;
}

ptr analysis::als_state::simplify_ptr_sum(vector<id_shared_t> const &pointers) {
  if(!pointers.size()) return ptr(special_ptr::badptr, vs_finite::single(0));
  optional<ptr> result;
  for(auto const &_ptr : pointers) {
    auto set_ptr = [&]() {
      if(!result)
        result = ptr(_ptr, vs_finite::single(0));
      else
        result = ptr(special_ptr::badptr, vs_finite::single(0));
    };
    summy::rreil::id_visitor idv;
    idv._([&](sm_id *sid) { set_ptr(); });
    idv._([&](special_ptr *sptr) {
      switch(sptr->get_kind()) {
        case NULL_PTR: {
          break;
        }
        case BAD_PTR: {
          result = ptr(special_ptr::badptr, vs_finite::single(0));
          break;
        }
      }
    });
    idv._default([&](id *__) { set_ptr(); });
    _ptr->accept(idv);
  }
  if(!result)
    return ptr(special_ptr::_nullptr, vs_finite::single(0));
  else
    return result.value();
}

api::num_expr *analysis::als_state::replace_pointers(api::num_expr *e) {
  num_expr *result;
  num_visitor nv;
  std::map<id_shared_t, id_shared_t, id_less_no_version> id_gen_map;
  nv._([&](num_expr_cmp *ec) { result = new num_expr_cmp(replace_pointers(id_gen_map, ec->get_opnd()), ec->get_op()); });
  nv._([&](num_expr_lin *el) { result = new num_expr_lin(replace_pointers(id_gen_map, el->get_inner())); });
  nv._([&](num_expr_bin *el) {
    result = new num_expr_bin(replace_pointers(id_gen_map, el->get_opnd1()), el->get_op(), replace_pointers(id_gen_map, el->get_opnd2()));
  });
  e->accept(nv);
  return result;
}

api::num_linear *analysis::als_state::replace_pointers(std::map<id_shared_t, id_shared_t, id_less_no_version> &id_gen_map, api::num_linear *l) {
  map<id_shared_t, int64_t, id_less_no_version> terms;
  num_linear *result;
  num_visitor nv;
  nv._([&](num_linear_term *lt) {
    lt->get_next()->accept(nv);
    auto e_it = elements.find(lt->get_var()->get_id());
    if(e_it != elements.end()) {
      id_set_t &aliases = e_it->second;
      if(aliases.size() == 1) {
        terms[*aliases.begin()] += lt->get_scale();
        terms[lt->get_var()->get_id()] += lt->get_scale();
      } else {
        auto gen_it = id_gen_map.find(lt->get_var()->get_id());
        if(gen_it == id_gen_map.end())
          tie(gen_it, ignore) = id_gen_map.insert(make_pair(lt->get_var()->get_id(), numeric_id::generate()));
        terms[gen_it->second] += lt->get_scale();
      }
    } else
      terms[lt->get_var()->get_id()] = lt->get_scale();
  });
  nv._([&](num_linear_vs *lvs) { result = lvs->copy(); });
  l->accept(nv);
  for(auto term_mapping : terms)
    result = new num_linear_term(term_mapping.second, new num_var(term_mapping.first), result);
  return result;
}

api::num_linear *analysis::als_state::replace_pointers(api::num_linear *l) {
  std::map<id_shared_t, id_shared_t, id_less_no_version> id_gen_map;
  return replace_pointers(id_gen_map, l);
}

als_state *analysis::als_state::domop(domain_state *other, size_t current_node, domopper_t domopper) {
  als_state const *other_casted = dynamic_cast<als_state *>(other);
  numeric_state *me_compat;
  numeric_state *other_compat;
  elements_t elements_compat;
  tie(ignore, elements_compat, me_compat, other_compat) = compat(this, other_casted);
  als_state *result = new als_state((me_compat->*domopper)(other_compat, current_node), elements_compat);
  delete me_compat;
  delete other_compat;
  return result;
}

void als_state::put(std::ostream &out) const {
  bool first = true;
  out << "{";
  for(auto &elem : elements) {
    id_shared_t id = elem.first;
    singleton_value_t const &aliases = elem.second;
    if(!first)
      out << ", ";
    else
      first = false;
    out << "P(" << *id << ") -> {";
    bool first = true;
    for(auto &alias : aliases) {
      if(first)
        first = false;
      else
        out << ", ";
      out << *alias;
    }
    out << "}";
  }
  out << "}" << endl;
  out << "Child state: {" << endl;
  out << *child_state;
  out << endl
      << "}";
}

analysis::als_state::als_state(numeric_state *child_state, elements_t elements)
    : child_state(child_state), elements(elements) {}

analysis::als_state::~als_state() {
  delete child_state;
}

void analysis::als_state::bottomify() {
  elements.clear();
  child_state->bottomify();
}

bool als_state::is_bottom() const {
  return child_state->is_bottom();
}

bool als_state::operator>=(const domain_state &other) const {
  als_state const &other_casted = dynamic_cast<als_state const &>(other);
  bool als_a_ge_b;
  numeric_state *me_compat;
  numeric_state *other_compat;
  tie(als_a_ge_b, ignore, me_compat, other_compat) = compat(this, &other_casted);
  bool child_ge = *me_compat >= *other_compat;
  delete me_compat;
  delete other_compat;
  return als_a_ge_b && child_ge;
}

als_state *als_state::join(domain_state *other, size_t current_node) {
  return domop(other, current_node, &numeric_state::join);
}

als_state *als_state::meet(domain_state *other, size_t current_node) {
  als_state const *other_casted = dynamic_cast<als_state *>(other);

  auto merge_map = [&](auto &dest, auto &a, auto &b) {
    for(auto &mapping_me : a) {
      auto mapping_other = b.find(mapping_me.first);
      if(mapping_other != b.end())
        throw string("analysis::equality_state::meet(): Case not implemented");
      else
        dest.insert(mapping_me);
    }
    for(auto &mapping_other : b) {
      auto mapping_me = a.find(mapping_other.first);
      if(mapping_me == a.end()) dest.insert(mapping_other);
    }
  };

  elements_t elements_new;
  merge_map(elements_new, elements, other_casted->elements);

  numeric_state *child_met = child_state->meet(other_casted->child_state, current_node);

  return new als_state(child_met, elements_new);
}

als_state *als_state::widen(domain_state *other, size_t current_node) {
  return domop(other, current_node, &numeric_state::widen);
}

als_state *als_state::narrow(domain_state *other, size_t current_node) {
  return domop(other, current_node, &numeric_state::narrow);
}

void als_state::assign(api::num_var *lhs, api::num_expr *rhs, bool strong) {
  //    cout << "assign expression in als_state: " << *lhs << " <- " << *rhs << endl;
  bool linear = false;
  num_visitor nv(true);
  nv._([&](num_expr_lin *le) { linear = true; });
  rhs->accept(nv);
  set<num_var *> _vars_rhs = ::vars(rhs);

  if(linear) {
    /*
     * Todo: What about scaled pointers? We need 'slice' from the paper
     */

    vector<id_set_t> aliases_vars_rhs;
    for(auto &var : _vars_rhs) {
      auto var_it = elements.find(var->get_id());
      if(var_it == elements.end())
        /*
         * Todo: Useless?
         */
        aliases_vars_rhs.push_back(id_set_t{special_ptr::_nullptr});
      else {
        assert(var_it->second.size() != 0);
        aliases_vars_rhs.push_back(var_it->second);
      }
      //      auto var_it = elements.find(var->get_id());
      //      if(var_it != elements.end()) aliases_rhs.insert(var_it->second.begin(), var_it->second.end());
    }
    if(aliases_vars_rhs.size() == 0) aliases_vars_rhs.push_back({special_ptr::_nullptr});

    vector<id_set_t::iterator> alias_iterators;
    for(size_t i = 0; i < aliases_vars_rhs.size(); i++)
      alias_iterators.push_back(aliases_vars_rhs[i].begin());

    id_set_t aliases_new;
    while(alias_iterators[0] != aliases_vars_rhs[0].end()) {
      vector<id_shared_t> aliases_current;
      for(auto &aliases_it : alias_iterators)
        aliases_current.push_back(*aliases_it);

      ptr alias_new_next = simplify_ptr_sum(aliases_current);
      assert(*alias_new_next.offset == vs_finite::single(0));
      aliases_new.insert(alias_new_next.id);

      for(size_t i = 1; i < alias_iterators.size(); i++) {
        alias_iterators[i]++;
        if(alias_iterators[i] == aliases_vars_rhs[i].end()) alias_iterators[i] = aliases_vars_rhs[i].begin();
      }
      alias_iterators[0]++;
    }

    if(strong) {
      elements[lhs->get_id()] = aliases_new;
      child_state->assign(lhs, rhs);
    } else {
      elements[lhs->get_id()].insert(aliases_new.begin(), aliases_new.end());
      child_state->weak_assign(lhs, rhs);
    }
  } else {
    /*
     * If the expression is not a linear expression, the result is a number and not a pointer. Therefore,
     * we replace pointers in the expression and re-offset the result to the null pointer.
     */

    num_expr *rhs_replaced = replace_pointers(rhs);
    id_set_t aliases_new = id_set_t{special_ptr::_nullptr};

    if(strong) {
      elements[lhs->get_id()] = aliases_new;
      child_state->assign(lhs, rhs_replaced);
    } else {
      elements[lhs->get_id()].insert(aliases_new.begin(), aliases_new.end());
      child_state->weak_assign(lhs, rhs_replaced);
    }

    delete rhs_replaced;
  }
}

void als_state::assign(api::num_var *lhs, api::ptr_set_t aliases) {
  aliases = normalise(aliases);
  optional<vs_shared_t> offset_joined;
  for(auto alias : aliases)
    if(offset_joined)
      offset_joined = value_set::join(offset_joined.value(), alias.offset);
    else
      offset_joined = alias.offset;
  num_expr *offset_e = new num_expr_lin(new num_linear_vs(offset_joined.value()));
  child_state->assign(lhs, offset_e);
  delete offset_e;

  id_set_t alias_set;
  for(auto alias : aliases)
    alias_set.insert(alias.id);
  elements[lhs->get_id()] = alias_set;
}

void als_state::assign(api::num_var *lhs, api::num_expr *rhs) {
  assign(lhs, rhs, true);
}

void als_state::weak_assign(api::num_var *lhs, api::num_expr *rhs) {
  assign(lhs, rhs, false);
}

void als_state::assume(api::num_expr_cmp *cmp) {
  if(is_bottom()) return;
  /*
   * Todo: Allow pointer comparisons; current implementation looses
   * aliasing information...
   *
   * Todo: Update paper: Ignoring assumptions in case they contain pointers
   * is not a viable option since now variables are pointers 'by default'
   */

  /*
   * Todo: Unsound / Uncool?!
   */
  //  cout << "bef: " << *this << endl;

  /*
   * Equality hack:
   *  - One variable: Something like 'A == 99', re-offset to null pointer
   *  - Todo: Two variables: Equate alias sets
   *  - Not one variable: Ignore test
   *
   * Future work:
   *  - Unhacking
   *  - If two alias sets with different addresses are assumed to be equal,
   *  the respective memeory regions need to be merged?!
   */

  auto _vars = ::vars(cmp);
  bool all_null = true;
  for(auto &var : _vars) {
    auto const &v_elems_it = elements.find(var->get_id());
    if(v_elems_it != elements.end()) {
      auto &v_elems = v_elems_it->second;
      /*
       * Todo: size() == 0?
       */
      if(v_elems.size() == 1 && (**v_elems.begin() == *special_ptr::_nullptr)) {
      } else {
        all_null = false;

        switch(cmp->get_op()) {
          case NEQ: {
            break;
          }
          case EQ: {
            if(_vars.size() != 1) break;
          }
          default: {
            elements[var->get_id()] = id_set_t {special_ptr::_nullptr};
            child_state->kill({var});
            break;
          }
        }
      }
    } else
      all_null = false;
  }
  switch(cmp->get_op()) {
    case NEQ: {
      if(all_null) child_state->assume(cmp);
      break;
    }
    case EQ: {
      if(_vars.size() != 1) {
        if(all_null) child_state->assume(cmp);
        break;
      }
    }
    default: {
      child_state->assume(cmp);
      break;
    }
  }

  //  cout << "after: " << *this << endl;
}

void als_state::assume(api::num_var *lhs, ptr_set_t aliases) {
  if(is_bottom()) return;

  aliases = normalise(aliases);

  if(aliases.size() == 0) {
    bottomify();
    return;
  }

  id_set_t aliases_ids;
  for(auto &alias : aliases)
    aliases_ids.insert(alias.id);

  auto aliases_iter = elements.find(lhs->get_id());
  if(aliases_iter == elements.end()) {
    /*
     * Top
     */
    elements[lhs->get_id()] = aliases_ids;
  } else {
    id_set_t aliases_lhs = aliases_iter->second;

    id_set_t aliases_new;
    set_intersection(aliases_ids.begin(), aliases_ids.end(), aliases_lhs.begin(), aliases_lhs.end(),
      inserter(aliases_new, aliases_new.begin()));

    if(aliases_new.size() == 0) {
      bottomify();
      return;
    } else
      elements[lhs->get_id()] = aliases_new;
  }

  optional<vs_shared_t> offset_aliases;
  for(auto &alias : aliases)
    if(offset_aliases)
      offset_aliases = value_set::join(offset_aliases.value(), alias.offset);
    else
      offset_aliases = alias.offset;

  num_linear *lhs_lin = new num_linear_term(lhs->copy());
  num_linear *oa_lin = new num_linear_vs(offset_aliases.value());
  num_expr_cmp *e = num_expr_cmp::equals(lhs_lin, oa_lin);
  child_state->assume(e);
  delete e;
  delete oa_lin;
  delete lhs_lin;
}

void als_state::kill(std::vector<api::num_var *> vars) {
  for(auto &var : vars) {
    //    cout << "ALS removing " << *var << endl;
    auto var_it = elements.find(var->get_id());
    if(var_it != elements.end()) elements.erase(var_it);
  }
  child_state->kill(vars);
}

void als_state::equate_kill(num_var_pairs_t vars) {
  /*
   * Build back map
   *
   * Todo: cache back map
   */
  elements_t back_map;
  for(auto e_mapping : elements)
    for(auto elem : e_mapping.second)
      back_map[elem].insert(e_mapping.first);

  for(auto var_pair : vars) {
    num_var *a, *b;
    tie(a, b) = var_pair;
    auto b_it = back_map.find(b->get_id());
    if(b_it != back_map.end())
      for(auto preimage : b_it->second) {
        auto &aliases = elements.at(preimage);
        //        assert(aliases_it != elements.end());
        //        id_set_t &aliases = *aliases_it;
        aliases.erase(b->get_id());
        aliases.insert(a->get_id());
      }
  }

  for(auto var_pair : vars) {
    num_var *a, *b;
    tie(a, b) = var_pair;
    auto b_it = elements.find(b->get_id());
    if(b_it != elements.end() && b_it->second.size() > 0) elements[a->get_id()] = b_it->second;
    elements.erase(b->get_id());
  }
  child_state->equate_kill(vars);
}

void als_state::fold(num_var_pairs_t vars) {
  child_state->fold(vars);
}

void als_state::copy_paste(api::num_var *to, api::num_var *from, numeric_state *from_state) {
  als_state *from_state_als = dynamic_cast<als_state*>(from_state);

  /*
   * We first insert and then overwrite, so 'to' is actually contained in the state
   */
//  assert(elements.find(to->get_id()) == elements.end());
  auto from_it = from_state_als->elements.find(from->get_id());
//  assert(from_it != from_state_als->elements.end());
  if(from_it != from_state_als->elements.end())
    elements[to->get_id()] = from_it->second;

  child_state->copy_paste(to, from, from_state_als->child_state);
}

api::ptr_set_t analysis::als_state::queryAls(api::num_var *nv) {
//    cout << "queryALS for " << *nv << endl;
//    cout << "offset: " << *child_state->queryVal(nv) << endl;
  ptr_set_t result;
  auto id_it = elements.find(nv->get_id());
  if(id_it == elements.end()) return child_state->queryAls(nv);
  singleton_value_t &aliases = id_it->second;
  for(auto alias : aliases) {
    if(*alias == *special_ptr::_nullptr) {
      auto child_aliases = child_state->queryAls(nv);
      result.insert(child_aliases.begin(), child_aliases.end());
    } else {
//      num_var *nv = new num_var(alias);
      vs_shared_t offset_bytes = child_state->queryVal(nv);
      //    vs_shared_t offset_bits = *vs_finite::single(8)*offset_bytes;
      result.insert(ptr(alias, offset_bytes));
//      delete nv;
    }
  }
  return result;
}

summy::vs_shared_t analysis::als_state::queryVal(api::num_linear *lin) {
  num_linear *replaced = replace_pointers(lin);
  vs_shared_t result = child_state->queryVal(replaced);
  delete replaced;
  return result;
}

summy::vs_shared_t analysis::als_state::queryVal(api::num_var *nv) {
  //  cout << "als_state::queryVal(" << *nv << ")" << endl;
  vs_shared_t child_value = child_state->queryVal(nv);
  auto alias_set_it = elements.find(nv->get_id());
  if(alias_set_it == elements.end()) return child_value;
  assert(alias_set_it->second.size() > 0);
  /*
   * Todo: no bottom                   ^------------------------
   */
  //  if(id_set_it->second.size() == 0)
  //    return value_set::bottom;
  vs_shared_t acc;
  bool first = true;
  for(auto alias : alias_set_it->second) {
    num_var *alias_var = new num_var(alias);
    vs_shared_t ptr_value = child_state->queryVal(alias_var);
    delete alias_var;
    vs_shared_t next = *child_value + ptr_value;
    if(first) {
      acc = next;
      first = false;
    } else
      acc = value_set::join(next, acc);
  }
  return acc;
}

bool analysis::als_state::cleanup(api::num_var *var) {
  bool child_clean = child_state->cleanup(var);
  if(elements.find(var->get_id()) != elements.end()) return true;
  /*
   * Todo: reverse map
   */
  for(auto &id_set_it : elements)
    if(id_set_it.second.find(var->get_id()) != id_set_it.second.end()) return true;
  return child_clean;
}

void analysis::als_state::project(api::num_vars *vars) {
  id_set_t const &p_ids = vars->get_ids();
  for(auto e_it = elements.begin(); e_it != elements.end();) {
    if(p_ids.find(e_it->first) == p_ids.end())
      elements.erase(e_it++);
    else {
      //      for(auto alias_it = e_it->second.begin(); alias_it != e_it->second.end();) {
      //        if(p_ids.find(*alias_it) == p_ids.end())
      //          e_it->second.erase(alias_it++);
      //        else
      //          ++alias_it;
      //      }
      ++e_it;
    }
  }

  child_state->project(vars);
}

api::num_vars *analysis::als_state::vars() {
  num_vars *child_vars = child_state->vars();

  id_set_t known_aliases;
  for(auto alias_mapping : elements)
    for(auto alias : alias_mapping.second)
      known_aliases.insert(alias);

  //  id_set_t vars_ids;
  //  set_union(child_ids.begin(), child_ids.end(), known_aliases.begin(), known_aliases.end(),
  //      inserter(vars_ids, vars_ids.begin()));
  //  delete child_vars;

  child_vars->add(known_aliases);

  return child_vars;
}

als_state *als_state::copy() const {
  return new als_state(*this);
}

api::ptr_set_t analysis::als_state::normalise(api::ptr_set_t aliases) {
  ptr_set_t result;
  for(auto &alias : aliases) {
    summy::rreil::id_visitor idv;
    idv._([&](sm_id *sid) {
      result.insert(ptr(special_ptr::_nullptr, *vs_finite::single((int64_t)sid->get_address()) + alias.offset));
    });
    idv._default([&](id *_id) {
      result.insert(alias);
    });
    alias.id->accept(idv);
  }
  return result;
}

std::tuple<bool, elements_t, numeric_state *, numeric_state *> analysis::als_state::compat(
  const als_state *a, const als_state *b) {
  bool als_a_ge_b = true;
  numeric_state *a_ = a->child_state->copy();
  numeric_state *b_ = b->child_state->copy();
  elements_t r;
  //  auto single = [&](id_shared_t id, numeric_state *n) {
  //    num_var *nv = new num_var(id);
  //    num_expr *top_expr = new num_expr_lin(new num_linear_vs(value_set::top));
  //    /*
  //     * Todo: more precision
  //     */
  //    n->assign(nv, top_expr);
  //    delete nv;
  //    delete top_expr;
  //  };
  auto compat_elements = [&](elements_t const &a_elements, elements_t const &b_elements,
    function<void(id_set_t const &, id_set_t const &, id_set_t const &)> joined_cb) {
    for(auto &x : a_elements) {
      assert(x.second.size() != 0);
      auto x_b_it = b_elements.find(x.first);

      id_set_t aliases_b;
      if(x_b_it == b_elements.end())
        aliases_b.insert(special_ptr::badptr);
      else {
        assert(x_b_it->second.size() > 0);
        aliases_b = x_b_it->second;
      }

      //      cout << "Join of" << endl;
      //      for(auto &u : x.second)
      //        cout << *u << " ";
      //      cout << endl << "and" << endl;
      //      for(auto &u : x_b_it->second)
      //        cout << *u << " ";
      //      cout << endl << "is" << endl;
      id_set_t joined;
      set_union(x.second.begin(), x.second.end(), aliases_b.begin(), aliases_b.end(), inserter(joined, joined.begin()));
      //      for(auto &u : joined)
      //        cout << *u << " ";
      //      cout << endl;
      r.insert(make_pair(x.first, joined));
      joined_cb(joined, x.second, aliases_b);
    }
  };
  compat_elements(
    a->get_elements(), b->get_elements(), [&](id_set_t const &joined, id_set_t const &a, id_set_t const &b) {
      if(joined.size() > a.size()) als_a_ge_b = false;
    });
  compat_elements(
    b->get_elements(), a->get_elements(), [&](id_set_t const &joined, id_set_t const &a, id_set_t const &b) {
      if(joined.size() > b.size()) als_a_ge_b = false;
    });
  return make_tuple(als_a_ge_b, r, a_, b_);
}
