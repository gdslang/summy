/*
 * mempath.h
 *
 *  Created on: Oct 29, 2015
 *      Author: Julian Kranz
 */

#pragma once
#include <cppgdsl/rreil/id/id.h>
#include <vector>
#include <memory>
#include <iostream>

class mempath {
public:
  struct step {
    int64_t offset;
    size_t size;

    bool operator <(const step &other) const;
  };
private:
  std::shared_ptr<gdsl::rreil::id> base;
  std::vector<step> path;

  int compare_to(const mempath &other) const;
public:
  mempath(std::shared_ptr<gdsl::rreil::id> base, std::vector<step> path);

  bool operator <(const mempath &other) const;
  bool operator ==(const mempath &other) const;
  friend std::ostream &operator<< (std::ostream &out, mempath const &_this);
};

std::ostream &operator<<(std::ostream &out, mempath const &_this);
