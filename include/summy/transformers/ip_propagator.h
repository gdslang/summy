/*
 * ip_propagator.h
 *
 *  Created on: Sep 21, 2014
 *      Author: jucs
 */

#ifndef IP_PROPAGATOR_H_
#define IP_PROPAGATOR_H_

#include "transformer.h"

class ip_propagator : public transformer {
  ip_propagator(cfg::cfg *cfg) :
      transformer(cfg) {
  }

  virtual void transform();
};


#endif /* IP_PROPAGATOR_H_ */
