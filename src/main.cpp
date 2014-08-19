#include <stdio.h>
#include <stdlib.h>
#include <cppgdsl/gdsl.h>
#include <cppgdsl/frontend/bare_frontend.h>
#include <cppgdsl/instruction.h>
#include <cppgdsl/rreil/rreil.h>

#include <vector>
#include <map>

#include <summy/cfg/cfg.h>
#include <summy/cfg/node.h>
#include <summy/cfg/edge.h>
#include <iostream>

using namespace gdsl::rreil;
using namespace std;

map<size_t, cfg::edge> next(size_t i) {
  map<size_t, cfg::edge> edge;
  edge[i + 1] = cfg::edge();
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

  for(statement *s : *rreil)
    delete s;
  delete rreil;

  vector<map<size_t, cfg::edge>> edges;
  edges.push_back(next(0));
  edges.push_back(next(1));
  edges.push_back(next(2));

  vector<cfg::node> nodes;

  cfg::cfg cfg(nodes, edges);

  cfg.dot(cout);

  return 0;
}
