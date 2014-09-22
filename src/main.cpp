#include <stdio.h>
#include <stdlib.h>
#include <cppgdsl/gdsl.h>
#include <cppgdsl/frontend/bare_frontend.h>
#include <cppgdsl/instruction.h>
#include <cppgdsl/rreil/rreil.h>

#include <vector>
#include <map>
#include <tuple>

#include <summy/cfg/cfg.h>
#include <summy/cfg/node.h>
#include <summy/cfg/edge.h>
#include <summy/cfg/bfs_iterator.h>
#include <iostream>
#include <fstream>

#include <summy/transformers/decomposer.h>
#include <summy/transformers/ip_propagator.h>

#include <cppgdsl/rreil/copy_visitor.h>

using namespace gdsl::rreil;
using namespace std;

int main(void) {
  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

  uint32_t buffer = 0x00d3e2d348;
  g.set_code((unsigned char*)&buffer, sizeof(buffer), 0);

  gdsl::instruction insn = g.decode();

  printf("Instruction: %s\n", insn.to_string().c_str());
  printf("---------------------------------\n");

  auto rreil = insn.translate();

  g.reset_heap();

  printf("RReil (no transformations):\n");
  for(statement *s : *rreil)
    printf("%s\n", s->to_string().c_str());

//  printf("RReil:\n");
//  for(statement *s : *rreil) {
//    copy_visitor v;
//    v._([&](int_t x) {
//      return new lin_imm(42);
//    });
//    s->accept(v);
//    s = v.get_statement();
//    printf("%s\n", s->to_string().c_str());
//    delete s;
//  }

  vector<tuple<uint64_t, vector<gdsl::rreil::statement*>*>> prog;
  prog.push_back(make_tuple(932, rreil));

  cfg::cfg cfg(prog);

  decomposer *d = new decomposer(&cfg);
  d->transform();
  delete d;

  ip_propagator *p = new ip_propagator(&cfg);
  p->transform();
  delete p;

//  printf("RReil (after transformations):\n");
//  for(statement *s : *rreil)
//    printf("%s\n", s->to_string().c_str());

  ofstream dot_fs;
  dot_fs.open("output.dot", ios::out);
  cfg.dot(dot_fs);
  dot_fs.close();

  for(auto stmt : *rreil)
    delete stmt;
  delete rreil;

  return 0;
}
