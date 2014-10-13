/*
 * util.cpp
 *
 *  Created on: Oct 9, 2014
 *      Author: Julian Kranz
 */

#include <summy/analysis/util.h>
//#include <cppgdsl/rreil/rreil.h>
//#include <cppgdsl/rreil/id/id_visitor.h>

#include <iostream>
#include <string>

//using namespace std;
//using namespace gdsl::rreil;

bool analysis::id_less::operator ()(std::shared_ptr<gdsl::rreil::id> a,
    std::shared_ptr<gdsl::rreil::id> b) {
//  cout << a->to_string() << " < " << b->to_string() << ": " << (a->to_string().compare(b->to_string()) < 0) << endl;
//  string a_str;
//  string b_str;
//
//  auto from_id = [&](std::shared_ptr<gdsl::rreil::id> _id) {
//    id_visitor iv;
//    string result = "";
//    iv._([&](_virtual *v) {
//      result = v->
//    });
//    return result;
//  };

  return a->to_string().compare(b->to_string()) < 0;
}
