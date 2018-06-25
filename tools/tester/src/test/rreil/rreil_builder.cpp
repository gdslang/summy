/*
 * rreil_builder.cpp
 *
 *  Created on: Mar 9, 2015
 *      Author: Julian Kranz
 */
#include <summy/test/rreil/rreil_builder.h>
#include <cppgdsl/rreil/id/arch_id.h>
#include <string>

using namespace std;
using namespace gdsl::rreil;

id_shared_t rreil_builder::temporary(string name) {
  return shared_ptr<id>(new arch_id(name));
}
