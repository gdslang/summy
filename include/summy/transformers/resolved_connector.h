/*
 * resolved_connector.h
 *
 *  Created on: Nov 26, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include "transformer.h"
#include <map>
extern "C" {
#include <gdsl_generic.h>
}

typedef std::map<cfg::edge_id, std::set<size_t>> edge_ass_t;

class resolved_connector: public transformer {
private:
  edge_ass_t resolved;
public:
  resolved_connector(cfg::cfg *cfg, edge_ass_t resolved) :
      transformer(cfg), resolved(resolved) {
  }
  resolved_connector(cfg::cfg *cfg, size_t root, edge_ass_t resolved) :
      transformer(cfg), resolved(resolved) {
  }

  virtual void transform();
};
