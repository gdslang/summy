/*
 * renamer.h
 *
 *  Created on: Oct 17, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/transformers/transformer.h>
#include <summy/analysis/adaptive_rd/adaptive_rd.h>

class renamer: public transformer {
public:
private:
  analysis::adaptive_rd::adaptive_rd_result *rd_result;
public:
  renamer(cfg::cfg *cfg, analysis::adaptive_rd::adaptive_rd_result *rd_result) :
      transformer(cfg), rd_result(rd_result) {
  }

  virtual void transform();
};
