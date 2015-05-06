/*
 * num_var.h
 *
 *  Created on: Feb 19, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/util.h>
#include <iosfwd>
#include <set>
#include <memory>

namespace analysis {
namespace api {

typedef std::shared_ptr<gdsl::rreil::id> id_shared_t;
typedef std::set<id_shared_t, id_less_no_version> id_set_t;

class num_var {
private:
  id_shared_t id;

public:
  num_var(id_shared_t id) : id(id) {
  }

  num_var *copy() {
    return new num_var(id);
  }

  id_shared_t get_id() const {
    return id;
  }

  friend std::ostream &operator<< (std::ostream &out, num_var &_this);
};

std::ostream& operator<<(std::ostream &out, num_var &_this);

class num_vars {
private:
  id_set_t ids;

public:
  num_vars(id_set_t ids) : ids(ids) {
  }

  num_vars *copy() {
    return new num_vars(ids);
  }

  id_set_t const &get_ids() const {
    return ids;
  }
};

}
}
