/*
 * rreil_builder.cpp
 *
 *  Created on: Mar 9, 2015
 *      Author: Julian Kranz
 */
#include <summy/test/rreil/rreil_builder.h>
#include <cppgdsl/rreil/id/virtual.h>

using namespace std;
using namespace gdsl::rreil;

id_shared_t rreil_builder::temporary() {
  static int_t i = 0;
  return shared_ptr<id>(new _virtual(i++));
}
