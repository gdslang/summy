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
#include <summy/transformers/goto_ip_adder.h>
#include <summy/transformers/ip_propagator.h>
#include <summy/transformers/trivial_connector.h>

#include <cppgdsl/rreil/copy_visitor.h>

using namespace gdsl::rreil;
using namespace std;

int main(void) {
  gdsl::bare_frontend f("current");
  gdsl::gdsl g(&f);

  uint32_t buffer = 0x0075c085;
  g.set_code((unsigned char*)&buffer, sizeof(buffer), 0);

  vector<tuple<uint64_t, vector<gdsl::rreil::statement*>*>> prog;
  for (int i = 0; i < 2; ++i) {
    gdsl::instruction insn = g.decode();

    printf("Instruction: %s\n", insn.to_string().c_str());
    printf("---------------------------------\n");

    auto rreil = insn.translate();

    g.reset_heap();

    printf("RReil (no transformations):\n");
    for(statement *s : *rreil)
      printf("%s\n", s->to_string().c_str());

    prog.push_back(make_tuple(i*2, rreil));
  }

  cfg::cfg cfg(prog);

  for(auto t : prog) {
    vector<gdsl::rreil::statement*>* rreil;
    tie(ignore, rreil) = t;
    for(auto stmt : *rreil)
      delete stmt;
    delete rreil;
  }

  vector<transformer*> transformers;
  transformers.push_back(new decomposer(&cfg));
  transformers.push_back(new goto_ip_adder(&cfg));
  transformers.push_back(new ip_propagator(&cfg));
  transformers.push_back(new trivial_connector(&cfg));
  for(auto t : transformers) {
    t->transform();
    delete t;
  }

//  printf("RReil (after transformations):\n");
//  for(statement *s : *rreil)
//    printf("%s\n", s->to_string().c_str());

  ofstream dot_fs;
  dot_fs.open("output.dot", ios::out);
  cfg.dot(dot_fs);
  dot_fs.close();



  return 0;
}
