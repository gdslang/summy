/*
 * ssa.cpp
 *
 *  Created on: Oct 27, 2014
 *      Author: Julian Kranz
 */

#include <summy/big_step/ssa.h>
#include <summy/analysis/adaptive_rd/adaptive_rd.h>
#include <summy/analysis/liveness/liveness.h>
#include <summy/transformers/ssa/phi_inserter.h>
#include <summy/transformers/ssa/renamer.h>

using namespace analysis;
using namespace liveness;
using namespace adaptive_rd;

void ssa::transform_analyzed() {
  {
    cfg::update_pop up = cfg.push_updates();

    phi_inserter *pi = new phi_inserter(&cfg, r.result());
    pi->transform();
    delete pi;

    fpl.notify(cfg.get_updates());
    fpr.notify(cfg.get_updates());

    renamer *ren = new renamer(&cfg, r.result());
    ren->transform();
    delete ren;
  }
}

ssa::ssa(cfg::cfg &cfg) :
    big_step(cfg), l(&cfg), fpl(&l), r(&cfg, l.result()), fpr(&r) {
}

void ssa::transduce() {
  fpl.iterate();
//  cout << l;

  fpr.iterate();
//  cout << r;

  transform_analyzed();
  cfg.register_observer(this);
}

void ssa::notify(const std::vector<cfg::update> &updates) {
  fpl.notify(updates);
  fpr.notify(updates);

  transform_analyzed();
}
