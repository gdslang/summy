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

void ssa::transduce_and_register() {
  fpl.iterate();
//  cout << l;

  fpr.iterate();
//  cout << r;

  {
    cfg::update_pop up = cfg.push_updates();

    pi.transform();

    /**
     * Propagate liveness / rd to nodes inserted by the phi
     * inserter.
     */
//    cout << endl << "<->" << endl << endl;
    fpl.notify(cfg.get_updates());
//    cout << endl << "<->" << endl << endl;
    fpr.notify(cfg.get_updates());

    {
      cfg::update_pop up = cfg.push_updates();

      ren.transform();

      fpl.notify(cfg.get_updates());
      fpr.notify(cfg.get_updates());
    }
  }

  cfg.register_observer(this);
}

void ssa::notify(const std::vector<cfg::update> &updates) {
  fpl.notify(updates);
  auto fpl_updates_first = cfg.adjacencies(fpl.get_updated());
  auto updates_combined = combine_updates(updates, fpl_updates_first);
  fpr.notify(updates_combined);
  auto fpr_updates_first = cfg.adjacencies(fpr.get_updated());
  {
    cfg::update_pop up = cfg.push_updates();

    pi.update(fpr_updates_first);

    /**
     * Propagate liveness / rd to nodes inserted by the phi
     * inserter.
     */
    fpl.notify(cfg.get_updates());
    auto fpl_updates_second = cfg.adjacencies(fpl.get_updated());
    auto updates_combined = combine_updates(cfg.get_updates(), fpl_updates_second);
    fpr.notify(updates_combined);
    auto fpr_updates_second = cfg.adjacencies(fpr.get_updated());

    {
      cfg::update_pop up = cfg.push_updates();

      ren.update(fpr_updates_first);
      ren.update(fpr_updates_second);

      fpl.notify(cfg.get_updates());
      fpr.notify(cfg.get_updates());
    }
  }
}
