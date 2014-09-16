/*
 * edge.h
 *
 *  Created on: Aug 19, 2014
 *      Author: jucs
 */

#pragma once

#include <stdint.h>
#include <iostream>
#include <cppgdsl/rreil/statement/statement.h>

namespace cfg {

class edge {
private:
  gdsl::rreil::statement *stmt;
public:
  edge(gdsl::rreil::statement *stmt) :
      stmt(stmt) {
  }
  ~edge();

  gdsl::rreil::statement *get_stmt() {
    return stmt;
  }

  void dot(std::ostream &stream);
};

}
