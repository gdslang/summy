/*
 * phi_inserter.h
 *
 *  Created on: Oct 17, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/transformers/transformer.h>
#include <summy/analysis/adaptive_rd/adaptive_rd.h>

class phi_inserter: public transformer {
private:
  analysis::adaptive_rd::adaptive_rd_result *rd_result;
public:
  phi_inserter(cfg::cfg *cfg, analysis::adaptive_rd::adaptive_rd_result *rd_result) :
      transformer(cfg), rd_result(rd_result) {
  }

  virtual void transform();
};
