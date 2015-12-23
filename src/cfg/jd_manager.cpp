/*
 * jd_manager.cpp
 *
 *  Created on: Apr 10, 2015
 *      Author: Julian Kranz
 */


#include <summy/cfg/jd_manager.h>
#include <summy/cfg/bfs_iterator.h>
#include <summy/cfg/node/address_node.h>
#include <summy/cfg/node/node.h>
#include <summy/cfg/node/node_visitor.h>
#include <functional>
#include <map>
#include <set>

using namespace cfg;
using namespace std;

jd_manager::jd_manager(::cfg::cfg *cfg) : addr(cfg), fp(&addr, *this) {
  fp.iterate();
  cfg->register_observer(&fp);
}

jump_dir jd_manager::jump_direction(size_t from, size_t to) {
  analysis::addr::addr_result ar = addr.result();
  auto from_address = ar.result.at(from)->get_address();
  auto to_address = ar.result.at(to)->get_address();
  if(!from_address || !to_address)
    return UNKNOWN;
  if(to_address.value() < from_address.value())
    return BACKWARD;
  else
    return FORWARD;
}
