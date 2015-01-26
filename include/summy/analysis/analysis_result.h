/*
 * analysis_result.h
 *
 *  Created on: Jan 26, 2015
 *      Author: Julian Kranz
 */

#pragma once

namespace analysis {

template<typename STATE_T>
struct analysis_result {
  STATE_T &result;

  analysis_result(STATE_T &result) : result(result) {
  }
};

}
