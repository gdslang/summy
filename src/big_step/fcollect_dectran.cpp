/*
 * dectran.cpp
 *
 *  Created on: Oct 30, 2014
 *      Author: Julian Kranz
 */

#include <summy/cfg/node/node_visitor.h>
#include <summy/cfg/node/address_node.h>
#include <summy/transformers/transformer.h>
#include <summy/transformers/decomposer.h>
#include <summy/transformers/goto_ip_adder.h>
#include <summy/transformers/ip_propagator.h>
#include <summy/transformers/trivial_connector.h>
#include <cppgdsl/gdsl.h>
#include <cppgdsl/block.h>
#include <cppgdsl/optimization.h>
#include <cppgdsl/instruction.h>
#include <cppgdsl/rreil/statement/statement.h>
#include <limits.h>
#include <summy/big_step/fcollect_dectran.h>
#include <vector>

using namespace gdsl::rreil;
using namespace std;
using namespace std::experimental;

size_t fcollect_dectran::initial_cfg(cfg::cfg &cfg, bool decode_multiple, std::experimental::optional<std::string> name) {
  size_t head_node = dectran::initial_cfg(cfg, decode_multiple, name);

  vector<transformer *> transformers;
  transformers.push_back(new decomposer(&cfg));
  transformers.push_back(new ip_propagator(&cfg));
  for(auto t : transformers) {
    t->transform();
    delete t;
  }

  /*
   * We're not interested in trivial updates...
   */
  cfg.clear_updates();

  return head_node;
}

fcollect_dectran::fcollect_dectran(gdsl::gdsl &gdsl, bool blockwise_optimized)
    : dectran(cfg, gdsl, blockwise_optimized), cfg() {

}

void fcollect_dectran::transduce() {
  initial_cfg(cfg, true, nullopt);
}
