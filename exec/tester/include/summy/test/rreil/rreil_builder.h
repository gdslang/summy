/*
 * rreil_builder.h
 *
 *  Created on: Mar 9, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <memory>
#include <cppgdsl/rreil/id/id.h>

typedef std::shared_ptr<gdsl::rreil::id> id_shared_t;

class rreil_builder {
public:
  static id_shared_t temporary();
};
