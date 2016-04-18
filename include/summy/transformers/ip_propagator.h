/*
 * ip_propagator.h
 *
 *  Created on: Sep 21, 2014
 *      Author: jucs
 */

#ifndef IP_PROPAGATOR_H_
#define IP_PROPAGATOR_H_

#include "transformer.h"
#include <cppgdsl/rreil/variable.h>
#include <cppgdsl/rreil/expr/expr.h>
#include <vector>
#include <tuple>

extern "C" {
#include <gdsl_generic.h>
}

class ip_propagator : public transformer {
private:
  bool speculative_decoding;
  std::tuple<bool, int_t> evaluate(int_t ip_value, gdsl::rreil::expr *e);
  std::vector<int_t> *analyze_ip();
public:
  ip_propagator(cfg::cfg *cfg, bool speculative_decoding) :
      transformer(cfg), speculative_decoding(speculative_decoding) {
  }
  ip_propagator(cfg::cfg *cfg, size_t root, bool speculative_decoding) :
      transformer(cfg, root), speculative_decoding(speculative_decoding) {
  }

  virtual void transform();
};

#endif /* IP_PROPAGATOR_H_ */
