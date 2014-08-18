/*
 * file_provider.h
 *
 *  Created on: May 8, 2014
 *      Author: Julian Kranz
 */

#pragma once

#include <summy/binary/stream_provider.h>

class file_provider : public stream_provider {
public:
  file_provider(char const *file);
  virtual ~file_provider();
};
