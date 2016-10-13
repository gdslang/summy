/*
 * Copyright 2014-2016 Julian Kranz, Technical University of Munich
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
        return new address_node(id, addr, DECODABLE);
      });

      auto ip_assign = make_assign(64, make_variable(make_id("IP"), 0),
          make_expr(make_sexpr(make_linear(addr))));
      cfg->update_edge(eid.from, new_addr_node, new stmt_edge(ip_assign.get()));
    }
  }
}
