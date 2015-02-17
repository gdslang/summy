#include <summy/big_step/dectran.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/node/address_node.h>
#include <summy/big_step/ssa.h>
#include <cppgdsl/gdsl.h>
#include <cppgdsl/frontend/bare_frontend.h>
#include <bjutil/binary/elf_provider.h>
#include <bjutil/printer.h>
#include <bjutil/gdsl_init.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <iosfwd>
#include <vector>
#include <map>
#include <tuple>
#include <iostream>
#include <fstream>
#include <functional>

#include <cvc4/expr/expr.h>
#include <cvc4/expr/expr_manager.h>
#include <cvc4/cvc4.h>
#include <summy/analysis/adaptive_rd/adaptive_rd.h>
#include <summy/analysis/fixpoint.h>
#include <summy/analysis/ismt/ismt.h>
#include <summy/analysis/liveness/liveness.h>
#include <summy/transformers/resolved_connector.h>

#include <summy/value_set/value_set.h>
#include <summy/value_set/vs_finite.h>
#include <summy/value_set/vs_open.h>
#include <summy/value_set/vs_top.h>

#include <cstdio>

using analysis::adaptive_rd::adaptive_rd;
using analysis::fixpoint;
using analysis::liveness::liveness;
using cfg::address_node;
using cfg::edge;

using namespace gdsl::rreil;
using namespace std;
using namespace CVC4;
using namespace analysis;
using namespace summy;

int main(int argc, char **argv) {
//  ExprManager em;
//  Expr a = em.mkVar("a", em.booleanType());

//  Expr Q = em.mkExpr(kind::)

//  ArrayType at = em.mkArrayType(em.integerType(), em.integerType());
//  Expr m0 = em.mkVar("m0", at);
//  Expr update = em.mkExpr(kind::STORE, arr, em.mkConst(Rational(0)), em.mkConst(Rational(42)));

//  Expr m0_update = em.mkExpr(kind::EQUAL, em.mkExpr(kind::SELECT, m0, em.mkConst(Rational(0))), em.mkConst(Rational(99)));
//  Expr m0_update_2 = em.mkExpr(kind::EQUAL, em.mkExpr(kind::SELECT, m0, em.mkConst(Rational(10))), em.mkConst(Rational(77)));
//
//  Expr m1 = em.mkVar("m1", at);
//  Expr m1_ini = em.mkExpr(kind::EQUAL, m1, em.mkExpr(kind::STORE, m0, em.mkConst(Rational(0)), em.mkConst(Rational(42))));
//
//  Expr r = em.mkExpr(kind::AND, m0_update, m0_update_2);
//  r = em.mkExpr(kind::AND, r, m1_ini);

//  Expr x = em.mkVar("x", em.integerType());
//  Expr y = em.mkVar("y", em.integerType());

//  Expr select_arr_x = em.mkExpr(kind::SELECT, arr, x);
//  Expr select_arr2_y = em.mkExpr(kind::SELECT, arr2, y);
//  Expr sel_eq = em.mkExpr(kind::EQUAL, select_arr_x, select_arr2_y);

//  Expr arr_eq = em.mkExpr(kind::EQUAL, m0, m1);

//  Expr forall = em.mkExpr(kind::FORALL, em.mkExpr(kind::BOUND_VAR_LIST, x), em.mkExpr(kind::EQUAL, y, em.mkExpr(kind::PLUS, x, em.mkConst(Rational(2)))));
//  Expr forall = em.mkExpr(kind::FORALL, em.mkExpr(kind::BOUND_VAR_LIST, x), em.mkExpr(kind::EQUAL, select_arr_x, em.mkExpr(kind::SELECT, arr, em.mkExpr(kind::PLUS, x, em.mkConst(Rational(2))))));
//  Expr forall = em.mkExpr(kind::FORALL, em.mkExpr(kind::BOUND_VAR_LIST, x), em.mkExpr(kind::EQUAL, x, em.mkConst(Rational(2))));
//  cout << forall << endl;

//  Expr r = em.mkExpr(kind::AND, update, forall);
//  r = em.mkExpr(kind::AND, r, sel_eq);

//  Expr r = em.mkExpr(kind::AND, update, arr_eq);

//  Expr sel_arr2_2 = em.mkExpr(kind::SELECT, m1, em.mkConst(Rational(0)));

//  Datatype value_type("value");
//
//  DatatypeConstructor val_ctor("VAL");
//  val_ctor.addArg("value", em.mkBitVectorType(64));
//  value_type.addConstructor(val_ctor);
//
//  DatatypeConstructor undef_ctor("UNDEF");
//  value_type.addConstructor(undef_ctor);
//
//  cout << value_type << endl;
//
//  Expr var = em.mkBoundVar("a", em.integerType());
//  Expr varList = em.mkExpr(kind::BOUND_VAR_LIST, var);
//  Expr body = em.mkExpr(kind::PLUS, var, em.mkConst(Rational(3)));
//  Expr _f = em.mkExpr(kind::LAMBDA, varList, body);
//
//  Expr application = em.mkExpr(kind::APPLY, _f, em.mkConst(Rational(5)));
//
//  Expr comp = em.mkVar("comp", em.integerType());
//
//  Expr r = em.mkExpr(kind::EQUAL, application, comp);
//
//  cout << r << endl;

//  DatatypeType rootType = em.mkDatatypeType(root);
//  cout << rootType << endl;
//
//  Expr A_true = em.mkExpr(kind::APPLY_CONSTRUCTOR, rootType.getDatatype()[0].getConstructor(), em.mkConst(true));
//  Expr B_ = em.mkExpr(kind::APPLY_CONSTRUCTOR, rootType.getDatatype()[1].getConstructor());
//
//  Expr a = em.mkVar("a", rootType);
//  Expr b = em.mkVar("b", rootType);
//  Expr c = em.mkVar("c", rootType);
//
//  Expr x = em.mkVar("x", em.booleanType());
//  Expr y = em.mkVar("y", em.booleanType());
//  Expr A_x = em.mkExpr(kind::APPLY_CONSTRUCTOR, rootType.getDatatype()[0].getConstructor(), x);
//  Expr A_y = em.mkExpr(kind::APPLY_CONSTRUCTOR, rootType.getDatatype()[0].getConstructor(), y);
//
//  Expr a_ini = em.mkExpr(kind::EQUAL, a, A_true);
//  Expr b_ini = em.mkExpr(kind::EQUAL, b, A_x);
//  Expr b_ini2 = em.mkExpr(kind::EQUAL, b, A_y);
//  Expr a_b = em.mkExpr(kind::DISTINCT, a, b);
//  Expr a_c = em.mkExpr(kind::DISTINCT, a, c);
//  Expr b_c = em.mkExpr(kind::DISTINCT, b, c);
//
//  Expr r = em.mkExpr(kind::AND, a_ini, b_ini);
////  r = em.mkExpr(kind::AND, r, a_b);
//  r = em.mkExpr(kind::AND, r, a_c);
////  r = em.mkExpr(kind::AND, r, b_c);
////  r = em.mkExpr(kind::AND, r, b_ini2);
//  r = em.mkExpr(kind::AND, r, em.mkExpr(kind::IFF, y, em.mkConst(false)));
//  r = em.mkExpr(kind::AND, r, em.mkExpr(kind::EQUAL, b, B_));

//  Expr A = em.mkConst(Constr(0, "A"));
//  Expr A_true;
//  {
//    std::vector<Expr> children;
//    children.push_back(A);
//    children.push_back(em.mkConst(Rational(1)));
//    A_true = em.mkExpr(kind::TUPLE, children);
//  }
//
//  Expr A_var;
//  Expr x = em.mkVar("x", em.integerType());
//  {
//    std::vector<Expr> children;
//    children.push_back(A);
//    children.push_back(x);
//    A_var = em.mkExpr(kind::TUPLE, children);
//  };
//
//  Expr A_var_y;
//  Expr y = em.mkVar("y", em.integerType());
//  {
//    std::vector<Expr> children;
//    children.push_back(A);
//    children.push_back(y);
//    A_var_y = em.mkExpr(kind::TUPLE, children);
//  };
//
////  Expr applied = em.mkExpr(kind::APPLY_CONSTR, v);
//  Expr a = em.mkVar("a", A_true.getType());
//  Expr b = em.mkVar("b", A_true.getType());
//  Expr c = em.mkVar("c", A_true.getType());
//
//  Expr a_ini = em.mkExpr(kind::EQUAL, a, A_true);
//  Expr b_ini = em.mkExpr(kind::EQUAL, b, A_var);
//  Expr c_ini = em.mkExpr(kind::EQUAL, c, A_var_y);
//
////  Expr b_ini = em.mkExpr(kind::IFF, b, em.mkConst(false));
//  Expr ini = em.mkExpr(kind::AND, a_ini, b_ini);
//  ini = em.mkExpr(kind::AND, ini, c_ini);
//  Expr a_eq_b = em.mkExpr(kind::DISTINCT, a, b);
//  Expr a_dis_c = em.mkExpr(kind::DISTINCT, a, c);
//
////  std::vector< std::pair<std::string, Type> > fields;
////  fields.push_back({"A", em.integerType()});
////  fields.push_back({"B", em.integerType()});
////  Expr foo = em.mkConst(Record(fields));
////  cout << foo << endl;
//
////  Expr x = em.mkExpr(kind::OR, a, em.mkExpr(kind::NOT, a));
//  Expr r = em.mkExpr(kind::AND, ini, a_eq_b);
//  r = em.mkExpr(kind::AND, r, a_dis_c);
//  r = em.mkExpr(kind::AND, r, em.mkExpr(kind::EQUAL, em.mkConst(Rational(100)), em.mkExpr(kind::PLUS, x, y)));
////  r = em.mkExpr(kind::AND, r, em.mkExpr(kind::EQUAL, em.mkConst(Rational(60)), em.mkExpr(kind::MINUS, x, y)));
//  SmtEngine smt(&em);

//  smt.setOption("check-models", SExpr("true"));
//  smt.setOption("produce-models", SExpr("true"));
//  smt.setOption("produce-assignments", SExpr("true"));
//
////  std::cout << x << " is " << smt.query(x) << std::endl;
//  std::cout << r << " is " << smt.checkSat(r) << std::endl;
////  for(auto blah : smt.getAssertions())
////    cout << blah << endl;
////  smt.getProof()->toStream(cout);
////  cout << smt.getAssignment() << endl;
//

//  cout << m0 << " := " << smt.getValue(m0) << endl;
//  cout << m1 << " := " << smt.getValue(m1) << endl;
//  cout << comp << " := " << smt.getValue(comp) << endl;
//  cout << "a := " << smt.getValue(a) << endl;
//  cout << "b := " << smt.getValue(b) << endl;
//  cout << "c := " << smt.getValue(c) << endl;
//  cout << "x := " << smt.getValue(x) << endl;
//
////  smt.addToAssignment(em.mkExpr(kind::DISTINCT, b, smt.getValue(b)));
//  Expr next = em.mkExpr(kind::DISTINCT, b, smt.getValue(b));
//  cout << next << endl;
//  smt.assertFormula(next);
//  std::cout << r << " is " << smt.checkSat(r) << std::endl;
//
//  cout << "a := " << smt.getValue(a) << endl;
//  cout << "b := " << smt.getValue(b) << endl;
//  cout << "c := " << smt.getValue(c) << endl;
//
//  next = em.mkExpr(kind::DISTINCT, b, smt.getValue(b));
//  cout << next << endl;
//  smt.assertFormula(next);
//  std::cout << r << " is " << smt.checkSat(r) << std::endl;
//
//  cout << "a := " << smt.getValue(a) << endl;
//  cout << "b := " << smt.getValue(b) << endl;
//  cout << "c := " << smt.getValue(c) << endl;
//  exit(0);

//  try {
//    auto foo = asm_compile("nop\nadd %rax, %rax");
//    for(auto &x : foo)
//      printf("%02x ", x);
//    printf("\n");
//
//  } catch(string &blah) {
//    cout << blah << endl;
//
//  }
  vs_shared_t s1 = make_shared<vs_finite>(vs_finite::elements_t {4, 2, 3});
  vs_shared_t s2 = make_shared<vs_finite>(vs_finite::elements_t {1, 9});

  vs_shared_t s3 = value_set::join(s1, s2);
  vs_shared_t s4 = value_set::widen(s1, s2);

  cout << *s4 << endl;

  exit(0);

  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

//  auto buffer = example(g, 0);
//  bj_gdsl bjg = gdsl_init_binfile(&f, "example.bin", 0);
  bj_gdsl bjg = gdsl_init_elf(&f, "a.out", ".text", "main", (size_t)1000);
//  bj_gdsl bjg = gdsl_init_immediate(&f, 0x00000000, 0);

//  dectran dt(*bjg.gdsl, false);
  dectran dt(*bjg.gdsl, false);
  dt.transduce();
  dt.register_();

  auto &cfg = dt.get_cfg();

  for(int i = 0; i < 1; i++) {
    cfg.commit_updates();

    ssa ssa(cfg);
    ssa.transduce();

//    if(i > 0)
//      break;

    ismt _ismt(&cfg, ssa.lv_result(), ssa.rd_result());

    edge_ass_t asses;

    for(auto &unres : dt.get_unresolved()) {
      ismt_edge_ass_t asses_loc = _ismt.analyse(unres);

      for(auto &mapping : asses_loc) {
        auto asses_it = asses.find(mapping.first);
        if(asses_it == asses.end()) asses[mapping.first] = mapping.second;
        else {
          set_union(asses_it->second.begin(), asses_it->second.end(), mapping.second.begin(), mapping.second.end(),
              inserter(asses_it->second, asses_it->second.end()));
        }
      }
    }

    for(auto &ass : asses)
      dt.resolve(ass.first.to);

    cout << print(asses, stream<cfg::edge_id>(), stream_printer<set<size_t>>()) << endl;

    ofstream ismt_fs;
    ismt_fs.open("ismt.dot", ios::out);
    _ismt.dot(ismt_fs);
    ismt_fs.close();

//    resolved_connector rc(&cfg, asses);
//    rc.transform();
  }

  ofstream dot_fs;
  dot_fs.open("output.dot", ios::out);
  cfg.dot(dot_fs);
  dot_fs.close();

//  g.set_code(NULL, 0, 0);
//  free(buffer);
  return 0;
}
