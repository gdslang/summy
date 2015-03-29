/*
 * num_var.h
 *
 *  Created on: Feb 19, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <summy/analysis/domains/numeric/numeric_state.h>
#include <iosfwd>

namespace analysis {
namespace api {

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

}
}
