/*
 * observer.cpp
 *
 *  Created on: Oct 23, 2014
 *      Author: Julian Kranz
 */

#include <summy/cfg/observer.h>
#include <summy/cfg/cfg.h>
#include <vector>
#include <assert.h>

using namespace std;

/*
 * observer
 */

cfg::observer::observer() {
}

cfg::observer::~observer() {
}

/*
 * recorder
 */

cfg::recorder::recorder(::cfg::cfg *cfg, bool record) : cfg(cfg), record(record) {
  cfg->register_observer(this);

  notify(cfg->get_updates());
}

cfg::recorder::~recorder() {
  cfg->unregister_observer(this);
}

void cfg::recorder::start() {
  record = true;
}

void cfg::recorder::stop() {
  record = false;
}

void cfg::recorder::notify(const std::vector<update> &updates) {
  if(!record)
    return;

  for(auto &update : updates) {
    switch(update.kind) {
      case INSERT:
      case UPDATE: {
        this->updates[update.from].insert(update.to);
        break;
      }
      case ERASE: {
        auto insertions_it = this->updates.find(update.from);
        if(insertions_it != this->updates.end()) {
          insertions_it->second.erase(update.to);
          if(insertions_it->second.size() == 0)
            this->updates.erase(insertions_it);
        }
        break;
      }
    }
  }
}

std::vector<cfg::update> cfg::recorder::checkout_updates() {
  vector<update> updates;
  for(auto node_updates_it : this->updates)
    for(auto node_update : node_updates_it.second)
      updates.push_back(update {UPDATE, node_updates_it.first, node_update});
  this->updates.clear();
  return updates;
}
