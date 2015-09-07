/*
 * bt_map.h
 *
 *  Created on: Sep 7, 2015
 *      Author: Julian Kranz
 */

#pragma once

#include <map>
#include <set>
#include <assert.h>

template <typename ELEM_T, typename CMP_T> class inverse_set_map {
private:
  typedef std::set<ELEM_T, CMP_T> set_t;
  typedef std::map<ELEM_T, set_t, CMP_T> map_t;
  map_t forward;
  map_t backward;

public:
  class modification_wrapper_t {
  private:
    map_t &backward;
    ELEM_T const &key;
    set_t &inner;

    void erase_back(set_t const &elements) {
      for(auto &e : elements) {
        auto backward_it = backward.find(e);
        assert(backward_it != backward.end());
        backward_it->second.erase(key);
        if(backward_it->second.size() == 0) backward.erase(backward_it);
      }
    }

    void update_back(set_t const &elements) {
      erase_back(inner);
      for(auto &e : elements)
        backward[e].insert(key);
    }

  public:
    modification_wrapper_t &operator=(set_t &&elements) {
      update_back(elements);
      inner = elements;
      return *this;
    }

    modification_wrapper_t &operator=(set_t &elements) {
      update_back(elements);
      inner = elements;
      return *this;
    }

    modification_wrapper_t &operator=(set_t const &elements) {
      update_back(elements);
      inner = elements;
      return *this;
    }

    operator ELEM_T const &() {
      return inner;
    }

    void insert(typename set_t::iterator from, typename set_t::iterator to) {
      for(auto it = from; it != to; it++)
        backward[*it].insert(key);
      inner.insert(from, to);
    }

    void insert(ELEM_T const &e) {
      backward[e].insert(key);
      inner.insert(e);
    }

    void erase(ELEM_T const &e) {
      if(inner.erase(e)) erase_back({e});
    }

    modification_wrapper_t(map_t &backward, ELEM_T const &key, set_t &inner)
        : backward(backward), key(key), inner(inner) {}
  };

  modification_wrapper_t operator[](ELEM_T const &key) {
    return modification_wrapper_t(backward, key, forward[key]);
  }

  modification_wrapper_t at(ELEM_T const &key) {
    return modification_wrapper_t(backward, key, forward.at(key));
  }

  typename map_t::const_iterator find(const ELEM_T &key) const {
    return forward.find(key);
  }

  //  typename map_t::iterator find(const ELEM_T &key) {
  //    return forward.find(key);
  //  }

  std::pair<typename map_t::const_iterator, bool> insert(std::pair<ELEM_T, set_t> const &p) {
    for(auto &e : std::get<1>(p))
      backward[e].insert(std::get<0>(p));
    return forward.insert(p);
  }

  typename map_t::const_iterator begin() const {
    return forward.begin();
  }

  //  typename map_t::iterator begin() {
  //    return forward.begin();
  //  }

  typename map_t::const_iterator end() const {
    return forward.end();
  }

  //  typename map_t::iterator end() {
  //    return forward.end();
  //  }

  void clear() {
    forward.clear();
    backward.clear();
  }

  void erase(ELEM_T const &e) {
    auto it = forward.find(e);
    if(it != forward.end()) erase(it);
  }

  void erase(typename map_t::const_iterator it) {
    for(auto const &e : it->second) {
      backward[e].erase(it->first);
    }
    forward.erase(it);
  }

  map_t const &get_backward() {
    return backward;
  }
};
