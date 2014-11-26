/*
 * resolved_connector.cpp
 *
 *  Created on: Nov 26, 2014
 *      Author: Julian Kranz
 */

#include <summy/transformers/resolved_connector.h>
#include <summy/cfg/cfg.h>
#include <summy/cfg/edge/edge.h>
#include <summy/cfg/node/address_node.h>
#include <cppgdsl/rreil/rreil.h>

using namespace gdsl::rreil;
using namespace cfg;

void resolved_connector::transform() {
  for(auto const &r : resolved) {
    edge_id const &eid = r.first;
//    auto &edges = *cfg->out_edge_payloads(eid.from());
    cfg->erase_destroy_edge(eid.from, eid.to);

    /*
     * Todo: Uncopy from trivial_connector
     */
    for(auto &addr : r.second) {
      size_t new_addr_node = cfg->create_node([&](size_t id) {
        return new address_node(id, addr, false);
      });

      auto ip_assign = new assign(64, new variable(new arch_id("IP"), 0),
          new expr_sexpr(new sexpr_lin(new lin_imm(addr))));

      cfg->update_edge(eid.from, new_addr_node, new stmt_edge(ip_assign));
    }
  }
}
