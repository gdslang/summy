/*
 * edge_visitor.h
 *
 *  Created on: Oct 17, 2014
 *      Author: Julian Kranz
 */

#pragma once
#include <functional>

namespace cfg {

class edge;
class stmt_edge;
class cond_edge;
class phi_edge;

class edge_visitor {
private:
  std::function<void(const edge*)> edge_callback = NULL;
  std::function<void(const stmt_edge*)> stmt_edge_callback = NULL;
  std::function<void(const cond_edge*)> cond_edge_callback = NULL;
  std::function<void(const phi_edge*)> phi_edge_callback = NULL;

public:
  virtual ~edge_visitor() {
  }

  virtual void visit(const edge *se) const {
    if(edge_callback != NULL) edge_callback(se);
    _default();
  }

  virtual void visit(const stmt_edge *se) const {
    if(stmt_edge_callback != NULL) stmt_edge_callback(se);
    _default();
  }

  virtual void visit(const cond_edge *se) const {
    if(cond_edge_callback != NULL) cond_edge_callback(se);
    _default();
  }

  virtual void visit(const phi_edge *se) const {
    if(phi_edge_callback != NULL) phi_edge_callback(se);
    _default();
  }

  virtual void _default() const {
  }

  void _(std::function<void(const edge*)> edge_callback) {
    this->edge_callback = edge_callback;
  }

  void _(std::function<void(const stmt_edge*)> stmt_edge_callback) {
    this->stmt_edge_callback = stmt_edge_callback;
  }

  void _(std::function<void(const cond_edge*)> cond_edge_callback) {
    this->cond_edge_callback = cond_edge_callback;
  }

  void _(std::function<void(const phi_edge*)> phi_edge_callback) {
    this->phi_edge_callback = phi_edge_callback;
  }
};

}
