#include <summy/big_step/dectran.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/node/address_node.h>
#include <summy/big_step/ssa.h>
#include <cppgdsl/gdsl.h>
#include <cppgdsl/frontend/bare_frontend.h>
#include <bjutil/binary/elf_provider.h>
#include <bjutil/print.h>
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

unsigned char *elf(gdsl::gdsl &g) {
  elf_provider elfp = [&]() {
    try {
      return elf_provider("a.out");
    } catch(string &s) {
      cout << "Error initializing elf provider: " << s << endl;
      throw string("no elf() :/");
    }
  }();
//  binary_provider::bin_range_t range = elfp.bin_range();
  binary_provider::entry_t dottext = elfp.section(".text");

  binary_provider::entry_t main;
  tie(ignore, main) = elfp.entry("main");

  unsigned char *buffer = (unsigned char*)malloc(dottext.size);
  memcpy(buffer, elfp.get_data().data + dottext.offset, dottext.size);
  g.set_code(buffer, (main.offset - dottext.offset) + main.size + 1000, dottext.address);
  if(g.seek(main.address))
    throw string(":/");
  cout << g.get_ip() << endl;

  return buffer;
}

unsigned char *manual(gdsl::gdsl &g, uint64_t ip) {
//  uint32_t buffer = 0x00ab48f3;
  uint32_t *buffer = (uint32_t*)malloc(sizeof(uint32_t));
  *buffer = 0x00e2d348;
  g.set_code((unsigned char*)buffer, sizeof(*buffer), ip);
  return (unsigned char*)buffer;
}

unsigned char *example(gdsl::gdsl &g, uint64_t ip) {
  unsigned char *buffer;
  size_t buffer_length;
  FILE *ms = open_memstream((char**)&buffer, &buffer_length);

  FILE *exf = fopen("example.bin", "r");
  while(!feof(exf)) {
    size_t n = 512;
    char loc[n];
    n = fread(loc, 1, n, exf);
    fwrite(loc, 1, n, ms);
  }
  fclose(exf);

  fclose(ms);
  g.set_code(buffer, buffer_length, ip);
  return buffer;
}

//template<typename X>
//printer<map<X, X>, X> blah(map<X, X> fuucppmap) {
//  auto pp = [](const X &a) {
//    return string("a");
//  };
//
//  return printer<map<X, X>, X>(fuucppmap, make_tuple(pp, pp));
//}

//template<typename X, typename Y, typename... T>
//printer<map<X, Y>, X, Y> blah(map<X, Y> fuucppmap, T... p) {
//  return printer<map<X, Y>, X, Y>(fuucppmap, p...);
//}


int main(void) {
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

//  Datatype root("root");
//
//  DatatypeConstructor A("A");
//  A.addArg("param", em.booleanType());
//  root.addConstructor(A);
//
//  DatatypeConstructor B("B");
//  root.addConstructor(B);
//
//  cout << root << endl;
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

////  smt.setOption("check-models", SExpr("true"));
//  smt.setOption("produce-models", SExpr("true"));
////  smt.setOption("produce-assignments", SExpr("true"));
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
//  cout << sel_arr2_2 << " := " << smt.getValue(sel_arr2_2) << endl;
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

  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

  auto buffer = elf(g);

  dectran dt(g, false);
  dt.transduce();
  dt.register_();

  auto &cfg = dt.get_cfg();
  cfg.commit_updates();

//  auto foo = cfg.out_edges(178)->at(179);
//  cfg.erase_edge(178, 179);
//  auto ani = cfg.create_node([&](size_t id) {
//    return new address_node(id, 7777);
//  });
//  cfg.update_edge(ani, 179, new edge());

  ssa ssa(cfg);
  ssa.transduce();

  ismt _ismt(&cfg, ssa.lv_result(), ssa.rd_result());
  for(auto &unres : dt.get_unresolved()) {
    ismt_edge_ass_t asses = _ismt.analyse(unres);

    /*
     * Todo: printable-Oberklasse, die automatisch string erzeugt aus Argument
     */

    function<std::string(const cfg::edge_id &ass)> edge_id_printer = [](const cfg::edge_id &eid) {
      return stream(eid)();
    };
    function<std::string(const set<size_t> &ass)> ass_printer = [](const set<size_t> &ass) {
      return stream(print(ass))();
    };

    cout << print(asses, stream<cfg::edge_id>(), ass_printer) << endl;
  }

  std::map<size_t, size_t> fuucppmap;
  fuucppmap[32] = 99;

  function<string(const size_t&)> pp = [](const size_t &a) -> string {
    return string("a");
  };

//  cout << blah(fuucppmap, make_tuple(pp, pp)) << endl;
  cout << print(fuucppmap) << endl;

//  cout << printer(fuucppmap, make_tuple(pp, pp)) << endl;

//  struct target {
//    int x;
//
//    target(int x) : x(x) {
//    }
//
//    bool operator <(const target &other) const {
//      return x < other.x;
//    }
//  };
//  set<target> fuu;
//  fuu.insert(target(1));
//  fuu.insert(target(42));
//  auto x = set_p<target>(fuu, [](const target &blah) {
//    return string(":-)");
//  });
//  cout << ": " << x << endl;

  ofstream ismt_fs;
  ismt_fs.open("ismt.dot", ios::out);
  _ismt.dot(ismt_fs);
  ismt_fs.close();

//  cfg.clear_updates();

//  cfg.update_edge(178, ani, foo);
//  cfg.commit_updates();

//  ofstream dot_fsb;
//  dot_fsb.open("output_before.dot", ios::out);
//  cfg.dot(dot_fsb);
//  dot_fsb.close();
//
//  cfg.commit_updates();

  ofstream dot_fs;
  dot_fs.open("output.dot", ios::out);
  cfg.dot(dot_fs);
  dot_fs.close();

  g.set_code(NULL, 0, 0);
  free(buffer);
  return 0;
}
