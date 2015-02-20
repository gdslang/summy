/*
 * num_var.h
 *
 *  Created on: Feb 19, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <cppgdsl/rreil/id/id.h>
#include <iosfwd>

namespace analysis {
namespace api {

class num_var {
private:
  gdsl::rreil::id *id;

public:
  num_var(gdsl::rreil::id *id) : id(id) {
  }

  gdsl::rreil::id *get_id() const {
    return id;
  }

  friend std::ostream &operator<< (std::ostream &out, num_var &_this);
};

std::ostream& operator<<(std::ostream &out, num_var &_this);

}
}
