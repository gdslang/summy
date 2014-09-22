/*
 * ip_propagator.h
 *
 *  Created on: Sep 21, 2014
 *      Author: jucs
 */

#ifndef IP_PROPAGATOR_H_
#define IP_PROPAGATOR_H_

#include "transformer.h"
#include <vector>

class ip_propagator : public transformer {
private:
  std::vector<size_t> *analyze_ip();
public:
  ip_propagator(cfg::cfg *cfg) :
      transformer(cfg) {
  }

  virtual void transform();
};


#endif /* IP_PROPAGATOR_H_ */
