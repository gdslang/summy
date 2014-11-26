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

using namespace gdsl::rreil;
using namespace std;

cfg::translated_program_t dectran::decode_translate(bool decode_multiple) {
  cfg::translated_program_t prog;
  if(gdsl.bytes_left() <= 0)
    throw string("Unable to decode");
  do {
    int_t ip = gdsl.get_ip();

    statements_t *rreil;
    if(blockwise_optimized) {
      gdsl::block b = gdsl.decode_translate_block(gdsl::preservation::CONTEXT, LONG_MAX);
      rreil = b.get_statements();
    } else {
      gdsl::instruction insn = gdsl.decode();
//      printf("Instruction: %s\n", insn.to_string().c_str());
//      printf("---------------------------------\n");
      rreil = insn.translate();
    }

    gdsl.reset_heap();

  //  printf("RReil (no transformations):\n");
  //  for(statement *s : *rreil)
  //    printf("%s\n", s->to_string().c_str());

    prog.push_back(make_tuple(ip, rreil));
  } while(decode_multiple && gdsl.bytes_left() > 0);

  return prog;
}

void dectran::initial_cfg(cfg::cfg &cfg, bool decode_multiple) {
  auto prog = decode_translate(decode_multiple);
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

  /*
   * We're not interested in trivial updates...
   */
  cfg.clear_updates();
}

dectran::dectran(gdsl::gdsl &gdsl, bool blockwise_optimized) :
    big_step(cfg), gdsl(gdsl), tc(&cfg), blockwise_optimized(blockwise_optimized) {
}

void dectran::transduce(bool decode_multiple) {
  initial_cfg(cfg, decode_multiple);
  tc.transform();
}

void dectran::notify(const std::vector<cfg::update> &updates) {
  auto up = cfg.push_updates();
  for(auto &update : updates) {
    if(update.kind != cfg::ERASE) {
      cfg::node_visitor nv;
      nv._([&](cfg::address_node *an) {
        size_t an_id = an->get_id();
        if(!an->get_decoded()) {
          cfg::cfg cfg_new;
          if(!gdsl.seek(an->get_address())) {
            initial_cfg(cfg_new, false);
            cfg.merge(cfg_new, an_id, 0 /* Todo: fix */);
            /*
             * From now on, 'an' is freed
             */
            tc.set_root(an_id);
            auto unres_new = tc.transform_ur();
            unresolved.insert(unres_new.begin(), unres_new.end());
          }
        }
      });
      cfg.get_node_payload(update.to)->accept(nv);
    }
  }
}
