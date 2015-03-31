/*
 * vsd_state.cpp
 *
 *  Created on: Feb 19, 2015
 *      Author: Julian Kranz
 */

#include <summy/analysis/domains/numeric/vsd_state.h>
#include <summy/analysis/domains/api/api.h>
#include <summy/value_set/value_set.h>
#include <summy/value_set/vs_finite.h>
#include <bjutil/printer.h>
#include <string>
#include <memory>

using namespace summy;
using namespace analysis;
using namespace analysis::api;
using namespace analysis::value_sets;

using namespace std;

void value_sets::vsd_state::put(std::ostream &out) const {
//  map<int, int*> a;
//  out << print(a, stream<int>(), stream_ptr<int*>());
//  out << print(elements, stream_ptr<singleton_key_t>(), stream_ptr<singleton_value_t>());
  if(is_bottom()) {
    cout << "⊥";
    return;
  }
  out << "{";
  bool first = true;
  for(auto &elem_it : elements) {
    if(first) first = false;
    else out << ", ";
    out << *elem_it.first << " -> " << *elem_it.second;
  }
  out << "}";
}

summy::vs_shared_t value_sets::vsd_state::lookup(id_shared_t id) {
  auto id_it = elements.find(id);
  if(id_it != elements.end()) return id_it->second;
  else return value_set::top;
}

bool analysis::value_sets::vsd_state::operator >=(domain_state const &other) const {
  vsd_state const &other_casted = dynamic_cast<vsd_state const&>(other);
  if(other_casted.is_bottom()) return true;
  else if(is_bottom()) return false;
  for(auto &mapping_mine : elements) {
    auto mapping_other = other_casted.elements.find(mapping_mine.first);
    if(mapping_other != other_casted.elements.end()) {
      if(!(*mapping_other->second <= mapping_mine.second)) return false;
    } else if(!(*mapping_mine.second == value_set::top)) return false;
  }
  return true;
}

vsd_state *analysis::value_sets::vsd_state::join(domain_state *other, size_t current_node) {
  vsd_state *other_casted = dynamic_cast<vsd_state*>(other);
  if(other_casted->is_bottom()) return new vsd_state(*this);
  else if(is_bottom()) return new vsd_state(*other_casted);

//  cout << *this << " ===JOIN=== " << *other_casted << endl;

  elements_t elems_new;
  auto join = [&](elements_t const &from, elements_t const &to) {
    for(auto &mapping_first : from) {
      auto mapping_second = to.find(mapping_first.first);
      if(mapping_second != to.end())
      elems_new[mapping_first.first] = value_set::join(mapping_first.second, mapping_second->second);
//        cout << "join(" << *mapping_first.second << ", " << *mapping_second->second << ") = " << *elems_new[mapping_first.first] << endl;
    }
  };
  join(elements, other_casted->elements);
  join(other_casted->elements, elements);

  return new vsd_state(elems_new);
}

vsd_state *analysis::value_sets::vsd_state::widen(domain_state *other, size_t current_node) {
  vsd_state *other_casted = dynamic_cast<vsd_state*>(other);
  if(other_casted->is_bottom()) return new vsd_state(*this);
  elements_t elements_new;
  for(auto &mapping_other : other_casted->elements)
    elements_new[mapping_other.first] = value_set::widen(lookup(mapping_other.first), mapping_other.second);
  return new vsd_state(elements_new);
}

vsd_state *analysis::value_sets::vsd_state::narrow(domain_state *other, size_t current_node) {
  vsd_state *other_casted = dynamic_cast<vsd_state*>(other);
  if(other_casted->is_bottom()) return new vsd_state(*this);
  elements_t elements_new;
  for(auto &mapping_other : other_casted->elements)
    elements_new[mapping_other.first] = value_set::narrow(lookup(mapping_other.first), mapping_other.second);
  return new vsd_state(elements_new);
}

vsd_state *analysis::value_sets::vsd_state::box(domain_state *other, size_t current_node) {
  if(*other <= *this) return this->narrow(other, current_node);
  else return this->widen(other, current_node);
}

void value_sets::vsd_state::assign(num_var *lhs, num_expr *rhs) {
  vs_shared_t er = queryVal(rhs);
  _is_bottom = _is_bottom || *er == value_set::bottom;
  if(is_bottom())
    return;
  elements[lhs->get_id()] = er;
}

void value_sets::vsd_state::weak_assign(num_var *lhs, num_expr *rhs) {
  vs_shared_t er = queryVal(rhs);
  _is_bottom = _is_bottom || *er == value_set::bottom;
  if(is_bottom())
    return;
  vs_shared_t current = queryVal(lhs);
  cout << "Join of " << *current << " and " << *er << endl;
  elements[lhs->get_id()] = value_set::join(current, er);
  cout << "^^^^^ " << *this;
}

void analysis::value_sets::vsd_state::assume(api::num_expr_cmp *cmp) {
  auto assume_zero = [&](vector<num_linear*> lins) {
    vector<vector<num_expr*>> fp_exprss;
    struct fp_lin {
      int64_t var_scale;
      num_linear *lin;
    };
    auto fp_term_for_lins = [&](vector<fp_lin> &fp_lins, num_linear *lin) {
      function<void(num_linear*, size_t)> fp_term_build;
      fp_term_build = [&](num_linear *lin, size_t count) {
        num_visitor nv;
        nv._([&](num_linear_term *lt) {
          fp_term_build(lt->get_next(), count + 1);
          for(size_t i = 0; i < fp_lins.size(); i++) {
            if(i == count)
              continue;
            fp_lins[i].lin = new num_linear_term(-lt->get_scale(),
              new num_var(lt->get_var()->get_id()), fp_lins[i].lin);
          }
          fp_lins[count].var_scale = lt->get_scale();
        });
        nv._([&](num_linear_vs *lvs) {
          for(size_t i = 0; i < count; i++)
            fp_lins.push_back({0, new num_linear_vs(-*lvs->get_value_set())});
        });
        lin->accept(nv);
      };
      fp_term_build(lin, 0);
    };
    for(auto &lin : lins) {
      vector<fp_lin> fp_lins;
      fp_term_for_lins(fp_lins, lin);
      vector<num_expr*> fp_exprs;
      for(fp_lin &fl: fp_lins)
        fp_exprs.push_back(new num_expr_bin(fl.lin, DIV, new num_linear_vs(vs_finite::single(fl.var_scale))));
//        fp_exprs.push_back(new num_expr_lin(l));
      fp_exprss.push_back(fp_exprs);
    }

    vector<num_var*> fp_vars;
    num_visitor nv(true);
    nv._([&](num_linear_term *lt) {
        fp_vars.push_back(lt->get_var());
        lt->get_next()->accept(nv);
    });
    lins[0]->accept(nv);

    vector<vs_shared_t> refined_vals = vector<vs_shared_t>(fp_vars.size());
    for(size_t i = 0; i < refined_vals.size(); i++)
      refined_vals[i] = value_set::top;
    bool change;
    do {
      change = false;
      for(size_t i = 0; i < fp_vars.size(); i++) {
        vs_shared_t current = queryVal(fp_vars[i]);

        vector<vs_shared_t> refineds;
        for(auto &fp_exprs : fp_exprss) {
          vs_shared_t refinement = queryVal(fp_exprs[i]);
//          cout << ">>> " << *current << " MEET " << *refinement << endl;
          refineds.push_back(value_set::meet(current, refinement));
        }
        vs_shared_t refined = value_set::bottom;
        for(auto re_cur : refineds)
          refined = value_set::join(refined, re_cur);
//        vs_shared_t refined = refineds[0];
//        cout << " = " << *refined << endl;
//        cout << *refined << endl;
        if(!(*refined_vals[i] <= refined)) {
          change = true;
          refined_vals[i] = refined;
          num_expr *expr = new num_expr_lin(new num_linear_vs(refined));
          assign(fp_vars[i], expr);
          delete expr;
        }
      }
    } while(change);

    for(auto &fp_exprs : fp_exprss)
      for(num_expr *e : fp_exprs)
        delete e;
  };

  switch(cmp->get_op()) {
    case EQ: {
      assume_zero({cmp->get_opnd()});
      break;
    }
    case NEQ: {
      num_linear *opnd_neg = cmp->get_opnd()->negate();
//      cout << "OPND: " << *cmp->get_opnd() << endl;
//      cout << "NEGATED: " << *opnd_neg << endl;
      num_linear *restriced_lower = converter::add(cmp->get_opnd(), make_shared<vs_open>(UPWARD, 1));
      num_linear *restriced_upper = converter::add(opnd_neg, make_shared<vs_open>(UPWARD, 1));
      assume_zero({restriced_lower, restriced_upper});
      delete opnd_neg;
      delete restriced_lower;
      delete restriced_upper;
      break;
    }
    case LE: {
      num_linear *restriced = converter::add(cmp->get_opnd(), make_shared<vs_open>(UPWARD, 0));
      assume_zero({restriced});
      delete restriced;
      break;
    }
    case LT: {
      num_linear *restriced = converter::add(cmp->get_opnd(), make_shared<vs_open>(UPWARD, 1));
      assume_zero({restriced});
      delete restriced;
      break;
    }
    case GE: {
      num_linear *restriced = converter::add(cmp->get_opnd(), make_shared<vs_open>(DOWNWARD, 0));
      assume_zero({restriced});
      delete restriced;
      break;
    }
    case GT: {
      num_linear *restriced = converter::add(cmp->get_opnd(), make_shared<vs_open>(DOWNWARD, -1));
      assume_zero({restriced});
      delete restriced;
      break;
    }
  }
}

void analysis::value_sets::vsd_state::assume(api::num_var *lhs, ptr_set_t aliases) {
  if(is_bottom())
    return;
//  throw string("analysis::value_sets::vsd_state::assume(num_var, ptr_set_t)");
}

void analysis::value_sets::vsd_state::kill(std::vector<api::num_var*> vars) {
  for(auto var : vars) {
    auto var_it = elements.find(var->get_id());
    if(var_it != elements.end()) elements.erase(var_it);
  }
}

void analysis::value_sets::vsd_state::equate_kill(num_var_pairs_t vars) {
  for(auto var_pair : vars) {
    num_var *a, *b;
    tie(a, b) = var_pair;
    elements[a->get_id()] = elements[b->get_id()];
    elements.erase(b->get_id());
  }
}

void analysis::value_sets::vsd_state::fold(num_var_pairs_t vars) {
  throw string("analysis::value_sets::vsd_state::assume(num_var_pairs_t)");
}

bool analysis::value_sets::vsd_state::cleanup(api::num_var *var) {
  if(*queryVal(var) == value_set::top) {
    elements.erase(var->get_id());
    return false;
  } else
    return true;
}

vsd_state *analysis::value_sets::vsd_state::bottom() {
  return new vsd_state(true);
}

vsd_state* analysis::value_sets::vsd_state::top() {
  return new vsd_state();
}

api::ptr_set_t analysis::value_sets::vsd_state::queryAls(api::num_var *nv) {
  return ptr_set_t { };
}

summy::vs_shared_t analysis::value_sets::vsd_state::queryVal(num_linear *lin) {
  num_visitor nv;
  vs_shared_t result;
  nv._([&](num_linear_term *lt) {
    vs_shared_t scale = vs_finite::single(lt->get_scale());
    vs_shared_t id = lookup(lt->get_var()->get_id());
    vs_shared_t next = queryVal(lt->get_next());
    result = *(*scale * id) + next;
  });
  nv._([&](num_linear_vs *lvs) {
    result = lvs->get_value_set();
  });
  lin->accept(nv);
  return result;
}

vs_shared_t value_sets::vsd_state::queryVal(num_expr *exp) {
  num_visitor nv;
  vs_shared_t result;
  nv._([&](num_expr_cmp *cmp) {
    result = value_set::top;
  });
  nv._([&](num_expr_lin *lin) {
    result = queryVal(lin->get_inner());
  });
  nv._([&](num_expr_bin *bin) {
    vs_shared_t opnd1 = queryVal(bin->get_opnd1());
    vs_shared_t opnd2 = queryVal(bin->get_opnd2());
    switch(bin->get_op()) {
      case MUL: {
        result = *opnd1 * opnd2;
        break;
      }
      case DIV: {
        result = *opnd1 / opnd2;
        break;
      }
      default: {
        result = value_set::top;
        break;
      }
    }
  });
  exp->accept(nv);
  return result;
}

summy::vs_shared_t analysis::value_sets::vsd_state::queryVal(api::num_var *nv) {
  return lookup(nv->get_id());
}

numeric_state *analysis::value_sets::vsd_state::copy() const {
  return new vsd_state(*this);
}
