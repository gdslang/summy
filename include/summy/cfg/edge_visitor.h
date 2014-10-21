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
  std::function<void(edge*)> edge_callback = NULL;
  std::function<void(stmt_edge*)> stmt_edge_callback = NULL;
  std::function<void(cond_edge*)> cond_edge_callback = NULL;
  std::function<void(phi_edge*)> phi_edge_callback = NULL;

public:
  virtual ~edge_visitor() {
  }

  virtual void visit(edge *se) {
    if(edge_callback != NULL) edge_callback(se);
    _default();
  }

  virtual void visit(stmt_edge *se) {
    if(stmt_edge_callback != NULL) stmt_edge_callback(se);
    _default();
  }

  virtual void visit(cond_edge *se) {
    if(cond_edge_callback != NULL) cond_edge_callback(se);
    _default();
  }

  virtual void visit(phi_edge *se) {
    if(phi_edge_callback != NULL) phi_edge_callback(se);
    _default();
  }

  virtual void _default() {
  }

  void _(std::function<void(edge*)> edge_callback) {
    this->edge_callback = edge_callback;
  }

  void _(std::function<void(stmt_edge*)> stmt_edge_callback) {
    this->stmt_edge_callback = stmt_edge_callback;
  }

  void _(std::function<void(cond_edge*)> cond_edge_callback) {
    this->cond_edge_callback = cond_edge_callback;
  }

  void _(std::function<void(phi_edge*)> phi_edge_callback) {
    this->phi_edge_callback = phi_edge_callback;
  }
};

}
