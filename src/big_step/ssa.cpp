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
#include <iostream>

using namespace std;
using namespace analysis;
using namespace liveness;
using namespace adaptive_rd;

ssa::ssa(cfg::cfg &cfg) :
    big_step(cfg), l(&cfg), fpl(&l), r(&cfg, l.result()), fpr(&r), pi(&cfg, r.result()), ren(&cfg, r.result()) {
}

void ssa::transduce() {
  fpl.iterate();
//  cout << l;

  fpr.iterate();
//  cout << r;

  {
    cfg::update_pop up = cfg.push_updates();

    pi.transform();

//    cout << endl << "<->" << endl << endl;
    fpl.notify(cfg.get_updates());
//    cout << endl << "<->" << endl << endl;

    fpr.notify(cfg.get_updates());

    ren.transform();
  }

  cfg.register_observer(this);
}

void ssa::notify(const std::vector<cfg::update> &updates) {
//  cout << endl << endl << "------------" << endl;
  fpl.notify(updates);
  fpr.notify(updates);
  {
    cfg::update_pop up = cfg.push_updates();

    auto adjacencies = cfg.adjacencies(fpr.get_updated());
    pi.update(adjacencies);

    fpl.notify(cfg.get_updates());
    fpr.notify(cfg.get_updates());

    auto adjacencies_new = cfg.adjacencies(fpr.get_updated());
    ren.update(adjacencies);
    ren.update(adjacencies_new);
  }
}
