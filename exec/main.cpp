#include <summy/big_step/dectran.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/node/address_node.h>
#include <summy/big_step/ssa.h>
#include <cppgdsl/gdsl.h>
#include <cppgdsl/frontend/bare_frontend.h>
#include <tardet/binary/elf_provider.h>

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

#include <cvc4/expr/expr.h>
#include <cvc4/expr/expr_manager.h>
#include <cvc4/cvc4.h>

using cfg::address_node;
using cfg::edge;

using namespace gdsl::rreil;
using namespace std;
using namespace CVC4;

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
  binary_provider::data_t data = elfp.get_data();
  binary_provider::entry_t e;
  tie(ignore, e) = elfp.entry("main");

  unsigned char *buffer = (unsigned char*)malloc(e.offset);
  memcpy(buffer, data.data + e.offset, e.size);
  g.set_code(buffer, e.size, e.address);
  return buffer;
}

unsigned char *manual(gdsl::gdsl &g, uint64_t ip) {
//  uint32_t buffer = 0x00ab48f3;
  uint32_t *buffer = (uint32_t*)malloc(sizeof(uint32_t));
  *buffer = 0x00e2d348;
  g.set_code((unsigned char*)buffer, sizeof(*buffer), ip);
  return (unsigned char*)buffer;
}

int main(void) {
  ExprManager em;
//  Expr a = em.mkVar("a", em.booleanType());

  Expr A = em.mkConst(Constr(0, "A"));
  Expr A_true;
  {
    std::vector<Expr> children;
    children.push_back(A);
    children.push_back(em.mkConst(true));
    A_true = em.mkExpr(kind::TUPLE, children);
  }

  Expr A_var;
  {
    std::vector<Expr> children;
    children.push_back(A);
    children.push_back(em.mkVar("x", em.booleanType()));
    A_var = em.mkExpr(kind::TUPLE, children);
  };

//  Expr applied = em.mkExpr(kind::APPLY_CONSTR, v);
  Expr a = em.mkVar("a", A_true.getType());
  Expr b = em.mkVar("b", A_true.getType());

  Expr a_ini = em.mkExpr(kind::EQUAL, a, A_true);
  Expr y = em.mkVar("y", em.booleanType());
  Expr b_ini = em.mkExpr(kind::EQUAL, b, A_var);
//  Expr b_ini = em.mkExpr(kind::IFF, b, em.mkConst(false));
  Expr ini = em.mkExpr(kind::AND, a_ini, b_ini);
  Expr a_eq_b = em.mkExpr(kind::EQUAL, a, b);

//  std::vector< std::pair<std::string, Type> > fields;
//  fields.push_back({"A", em.integerType()});
//  fields.push_back({"B", em.integerType()});
//  Expr foo = em.mkConst(Record(fields));
//  cout << foo << endl;

//  Expr x = em.mkExpr(kind::OR, a, em.mkExpr(kind::NOT, a));
  Expr x = em.mkExpr(kind::AND, ini, a_eq_b);
  SmtEngine smt(&em);

//  smt.setOption("check-models", SExpr("true"));
  smt.setOption("produce-models", SExpr("true"));
//  smt.setOption("produce-assignments", SExpr("true"));

//  std::cout << x << " is " << smt.query(x) << std::endl;
  std::cout << x << " is " << smt.checkSat(x) << std::endl;
//  for(auto blah : smt.getAssertions())
//    cout << blah << endl;
//  smt.getProof()->toStream(cout);
//  cout << smt.getAssignment() << endl;
  cout << smt.getValue(b) << endl;
  return 0;

  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

  auto buffer = elf(g);

  dectran dt(g, false);
  dt.transduce_and_register();

  auto &cfg = dt.get_cfg();
  cfg.commit_updates();

//  auto foo = cfg.out_edges(178)->at(179);
//  cfg.erase_edge(178, 179);
//  auto ani = cfg.create_node([&](size_t id) {
//    return new address_node(id, 7777);
//  });
//  cfg.update_edge(ani, 179, new edge());
//

  ssa ssa(cfg);
  ssa.transduce_and_register();
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
