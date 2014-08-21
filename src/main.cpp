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

using namespace gdsl::rreil;
using namespace std;

int main(void) {
  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

  uint32_t buffer = 0x00c1e910;
  g.set_code((char*)&buffer, sizeof(buffer), 0);

  gdsl::instruction insn = g.decode();

  printf("Instruction: %s\n", insn.to_string().c_str());
  printf("---------------------------------\n");

  auto rreil = insn.translate();

  g.reset_heap();

  printf("RReil:\n");
  for(statement *s : *rreil)
    printf("%s\n", s->to_string().c_str());

  vector<tuple<uint64_t, vector<gdsl::rreil::statement*>*>> prog;
  prog.push_back(make_tuple(932, rreil));

  cfg::cfg cfg(prog);

  ofstream dot_fs;
  dot_fs.open("output.dot", ios::out);

  cfg.dot(dot_fs);

  dot_fs.close();

  for(auto a : cfg)
    printf("id: %zu\n", a->get_id());

  for(auto stmt : *rreil)
    delete stmt;
  delete rreil;

  return 0;
}
