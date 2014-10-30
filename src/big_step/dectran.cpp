/*
 * dectran.cpp
 *
 *  Created on: Oct 30, 2014
 *      Author: Julian Kranz
 */


#include <summy/big_step/dectran.h>
#include <summy/cfg/node/node_visitor.h>
#include <summy/cfg/node/address_node.h>
#include <summy/transformers/transformer.h>
#include <summy/transformers/decomposer.h>
#include <summy/transformers/goto_ip_adder.h>
#include <summy/transformers/ip_propagator.h>
#include <summy/transformers/trivial_connector.h>
#include <cppgdsl/gdsl.h>
#include <cppgdsl/block.h>
#include <cppgdsl/instruction.h>
#include <cppgdsl/rreil/statement/statement.h>
#include <limits.h>
#include <vector>

using gdsl::rreil::statement;
using namespace std;

cfg::translated_program_t dectran::decode_translate() {
  cfg::translated_program_t prog;
  while(gdsl.bytes_left() > 0) {
    int_t ip = gdsl.get_ip();

//    gdsl::instruction insn = gdsl.decode();
//    printf("Instruction: %s\n", insn.to_string().c_str());
//    printf("---------------------------------\n");
//    auto rreil = insn.translate();
    gdsl::block b = gdsl.decode_translate_block(gdsl::preservation::CONTEXT, LONG_MAX);
    auto rreil = b.get_statements();

    gdsl.reset_heap();

    printf("RReil (no transformations):\n");
    for(statement *s : *rreil)
      printf("%s\n", s->to_string().c_str());

    prog.push_back(make_tuple(ip, rreil));

    break;
  }

  return prog;
}

void dectran::initial_cfg(cfg::cfg& cfg) {
  auto prog = decode_translate();
  cfg.add_program(prog);

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
//  transformers.push_back(new trivial_connector(&cfg));
  for(auto t : transformers) {
    t->transform();
    delete t;
  }
}

dectran::dectran(gdsl::gdsl &gdsl) : big_step(cfg), gdsl(gdsl)  {
}

void dectran::transduce_and_register() {
  initial_cfg(cfg);
  /*
   * Todo: trivial_connector must not traverse the whole graph!
   */
  trivial_connector tc(&cfg);
  tc.transform();
  cfg.register_observer(this);
}

void dectran::notify(const std::vector<cfg::update> &updates) {
  auto up = cfg.push_updates();
  for(auto &update : updates) {
    if(update.kind != cfg::ERASE) {
      cfg::node_visitor nv;
      nv._([&](cfg::address_node *an) {
        if(!an->get_decoded()) {
          cfg::cfg cfg_new;
          if(!gdsl.seek(an->get_address())) {
            initial_cfg(cfg_new);
            cfg.merge(cfg_new, an->get_id(), 0 /* Todo: fix */);

          }
        }
      });
      cfg.get_node(update.to)->accept(nv);
    }
  }
  /*
   * Todo: trivial_connector must not traverse the whole graph!
   */
  trivial_connector tc(&cfg);
  tc.transform();
}
