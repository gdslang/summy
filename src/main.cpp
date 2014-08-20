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
#include <iostream>
#include <fstream>

using namespace gdsl::rreil;
using namespace std;

map<size_t, cfg::edge*> next(size_t i, statement *stmt) {
  map<size_t, cfg::edge*> edge;
  edge[i + 1] = new cfg::edge(stmt);
  return edge;
}

int main(void) {
  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

  uint16_t buffer = 0x0000;
  g.set_code((char*)&buffer, sizeof(buffer), 0);

  gdsl::instruction insn = g.decode();

  printf("Instruction: %s\n", insn.to_string().c_str());
  printf("---------------------------------\n");

  auto rreil = insn.translate();

  g.reset_heap();

  printf("RReil:\n");
  for(statement *s : *rreil)
    printf("%s\n", s->to_string().c_str());

  vector<map<size_t, cfg::edge*>> edges;
  for(size_t i = 0; i < rreil->size(); i++) {
    edges.push_back(next(i, rreil->operator [](i)));
  }

  vector<tuple<uint64_t, vector<gdsl::rreil::statement*>*>> prog;
  prog.push_back(make_tuple(932, rreil));

  cfg::cfg cfg(prog);

  ofstream dot_fs;
  dot_fs.open("output.dot", ios::out);

  cfg.dot(dot_fs);

  dot_fs.close();

  for(statement *s : *rreil)
    delete s;
  delete rreil;

  return 0;
}
