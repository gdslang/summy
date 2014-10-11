/*
 * util.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/util.h>

#include <iostream>

using namespace std;

bool analysis::id_less::operator ()(std::shared_ptr<gdsl::rreil::id> a,
    std::shared_ptr<gdsl::rreil::id> b) {
//  cout << a->to_string() << " < " << b->to_string() << ": " << (a->to_string().compare(b->to_string()) < 0) << endl;
  return a->to_string().compare(b->to_string()) < 0;
}
