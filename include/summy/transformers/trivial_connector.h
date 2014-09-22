/*
 * trivial_connector.h
 *
 *  Created on: Sep 22, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include "transformer.h"

#include <map>

class trivial_connector : public transformer {
public:
  typedef std::map<size_t, size_t> start_node_map_t;
private:
  start_node_map_t start_node_map();
public:
  trivial_connector(cfg::cfg *cfg) :
      transformer(cfg) {
  }

  virtual void transform();
};
