/*
 * observer.h
 *
 *  Created on: Oct 23, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <vector>
#include <map>
#include <set>

namespace cfg {

struct update;
class cfg;

class observer {
public:
  observer();
  virtual ~observer();

  virtual void notify(std::vector<update> const &updates) = 0;
};

/*
 * Todo: Update kind (update/insert)
 */

class recorder : public observer {
private:
  ::cfg::cfg *cfg;
  std::map<size_t, std::set<size_t>> updates;
public:
  recorder(::cfg::cfg *cfg);
  ~recorder();

  void notify(std::vector<update> const &updates);
  std::vector<update> get_updates();
};

}


