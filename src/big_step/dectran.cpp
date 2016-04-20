/*
 * dectran.cpp
 *
 *  Created on: Oct 30, 2014
 *      Author: Julian Kranz
 */

#include <cppgdsl/gdsl_exception.h>
#include <summy/cfg/node/node_visitor.h>
#include <summy/cfg/node/address_node.h>
#include <summy/transformers/transformer.h>
#include <summy/transformers/decomposer.h>
#include <summy/transformers/goto_ip_adder.h>
#include <summy/transformers/ip_propagator.h>
#include <summy/transformers/trivial_connector.h>
#include <cppgdsl/gdsl.h>
#include <cppgdsl/gdsl_exception.h>
#include <cppgdsl/block.h>
#include <cppgdsl/optimization.h>
#include <cppgdsl/instruction.h>
#include <cppgdsl/rreil/statement/statement.h>
#include <limits.h>
#include <summy/big_step/dectran.h>
#include <vector>

using gdsl::gdsl_exception;

using namespace gdsl::rreil;
using namespace std;
using namespace std::experimental;

dec_interval::dec_interval(int_t from, int_t to) : from(from), to(to) {

}

bool dec_interval::operator <(const dec_interval &other) {
  return from < other.from;
}

cfg::translated_program_t dectran::decode_translate(bool decode_multiple) {
  cfg::translated_program_t prog;
  if(gdsl.bytes_left() <= 0) throw string("Unable to decode");
  do {
    int_t ip = gdsl.get_ip();

    statements_t *rreil;
    if(blockwise_optimized) {
      gdsl::block b = gdsl.decode_translate_block(gdsl::optimization_configuration::CONTEXT |
                                                    gdsl::optimization_configuration::LIVENESS |
                                                    gdsl::optimization_configuration::FSUBST,
        LONG_MAX);
      rreil = b.get_statements();
    } else {
      optional<gdsl::instruction> insn = [&]() -> optional<gdsl::instruction> {
        try {
          return optional<gdsl::instruction>(gdsl.decode());
        } catch(gdsl_exception &s) {
          cout << "Decoding error @0x" << hex << gdsl.get_ip() << dec << ": " << s << endl;
          gdsl.reset_heap();
          return nullopt;
        }
      }();
      if(!insn) continue;
      //            printf("Instruction: %s\n", insn.to_string().c_str());
      //            printf("---------------------------------\n");
      rreil = insn.value().translate();
    }

    gdsl.reset_heap();

    //    printf("RReil (no transformations):\n");
    //    for(statement *s : *rreil)
    //      printf("%s\n", s->to_string().c_str());

    prog.push_back(make_tuple(ip, rreil));
  } while(decode_multiple && gdsl.bytes_left() > 0);

  return prog;
}

size_t dectran::initial_cfg(cfg::cfg &cfg, bool decode_multiple, std::experimental::optional<std::string> name) {
  auto prog = decode_translate(decode_multiple);
  size_t head_node = cfg.add_program(prog, name);

  for(auto t : prog) {
    vector<gdsl::rreil::statement *> *rreil;
    tie(ignore, rreil) = t;
    for(auto stmt : *rreil)
      delete stmt;
    delete rreil;
  }

  //  vector<transformer *> transformers;
  //  transformers.push_back(new decomposer(&cfg, head_node));
  //  transformers.push_back(new goto_ip_adder(&cfg, head_node));
  //  transformers.push_back(new ip_propagator(&cfg, head_node));
  //  //  transformers.push_back(new trivial_connector(&cfg));
  //  for(auto t : transformers) {
  //    t->transform();
  //    delete t;
  //  }
  //
  //  /*
  //   * We're not interested in trivial updates...
  //   */
  //  cfg.clear_updates();

  return head_node;
}

dectran::dectran(cfg::cfg &cfg, gdsl::gdsl &gdsl, bool blockwise_optimized, bool speculative_decoding)
    : cfg(cfg), blockwise_optimized(blockwise_optimized), gdsl(gdsl), speculative_decoding(speculative_decoding) {}
