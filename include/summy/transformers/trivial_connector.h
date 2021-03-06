/*
 * trivial_connector.h
 *
 *  Created on: Sep 22, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include "transformer.h"
#include <map>
extern "C" {
#include <gdsl_generic.h>
}

class trivial_connector : public transformer {
public:
  typedef std::map<int_t, size_t> address_node_map_t;
private:
  /*
   * Map addresses to their respective address nodes
   */
  trivial_connector::address_node_map_t address_node_map;

  void update_address_node_map();
public:
  using transformer::transformer;

  std::set<size_t> transform_ur();
  virtual void transform();
};
