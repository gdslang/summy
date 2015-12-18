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
#include <summy/big_step/analysis_dectran.h>
#include <vector>

using namespace gdsl::rreil;
using namespace std;
using namespace std::experimental;

size_t analysis_dectran::initial_cfg(
  cfg::cfg &cfg, bool decode_multiple, std::experimental::optional<std::string> name) {
  size_t head_node = dectran::initial_cfg(cfg, decode_multiple, name);

  vector<transformer *> transformers;
  transformers.push_back(new decomposer(&cfg, head_node));
  transformers.push_back(new goto_ip_adder(&cfg, head_node));
  transformers.push_back(new ip_propagator(&cfg, head_node));
  //  transformers.push_back(new trivial_connector(&cfg));
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

analysis_dectran::analysis_dectran(gdsl::gdsl &gdsl, bool blockwise_optimized)
    : dectran(cfg, gdsl, blockwise_optimized), big_step::big_step(cfg), tc(&cfg), cfg() {}

void analysis_dectran::transduce(bool decode_multiple, std::experimental::optional<std::string> function_name) {
  size_t head_node = initial_cfg(cfg, decode_multiple, function_name);
  tc.set_root(head_node);
  tc.transform();
  f_heads.insert(head_node);
}

void analysis_dectran::transduce_function(size_t address, std::experimental::optional<std::string> function_name) {
  if(gdsl.seek(address)) {
    throw string("Unable to seek to function");
  }
  transduce(false, function_name);
}

void analysis_dectran::notify(const std::vector<cfg::update> &updates) {
  auto up = cfg.push_updates();
  for(auto &update : updates) {
    if(update.kind != cfg::ERASE) {
      cfg::node_visitor nv;
      nv._([&](cfg::address_node *an) {
        size_t an_id = an->get_id();
        if(an->get_decs() == cfg::DECODABLE) {
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
