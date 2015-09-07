/*
 * bt_map.h
 *
 *  Created on: Sep 7, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <map>
#include <set>

template<typename ELEM_T, typename CMP_T>
class inverse_set_map {
private:
  typedef std::set<ELEM_T, CMP_T> set_t;
  typedef std::map<ELEM_T, set_t, CMP_T> map_t;
  map_t forward;
  map_t backward;

public:
  struct modification_wrapper_t {
    set_t& inner;

    modification_wrapper_t& operator=(set_t &&e) {
      inner = e;
      return *this;
    }

    modification_wrapper_t& operator=(set_t &e) {
      inner = e;
      return *this;
    }

    operator ELEM_T const&() {
      return inner;
    }

    void insert(typename set_t::iterator from, typename set_t::iterator to) {
      inner.insert(from, to);
    }

    void insert(ELEM_T const &e) {
      inner.insert(e);
    }
  };

  modification_wrapper_t operator[] (ELEM_T &&key) {
    return modification_wrapper_t { forward[key] };
  }

  modification_wrapper_t operator[] (ELEM_T &key) {
    return modification_wrapper_t { forward[key] };
  }

  set_t &at(ELEM_T &key) {
    return forward.at(key);
  }

  typename map_t::const_iterator find(const ELEM_T &key) const {
    return forward.find(key);
  }

  typename map_t::iterator find(const ELEM_T &key) {
    return forward.find(key);
  }

  std::pair<typename map_t::iterator, bool> insert(std::pair<ELEM_T, set_t> const &p) {
    return forward.insert(p);
  }

  typename map_t::const_iterator begin() const {
    return forward.begin();
  }

  typename map_t::iterator begin() {
    return forward.begin();
  }

  typename map_t::const_iterator end() const {
    return forward.end();
  }

  typename map_t::iterator end() {
    return forward.end();
  }

  void clear() {
    forward.clear();
  }

  void erase(ELEM_T &&e) {
    forward.erase(e);
  }

  void erase(typename map_t::const_iterator it) {
//    for(auto &e : it.second) {
//      backward[e].erase(it.first);
//    }
    forward.erase(it);
  }
};
