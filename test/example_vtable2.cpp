/*
 * example_vtable0.cpp
 *
 *  Created on: Aug 5, 2014
 *      Author: jucs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>

class Super {
public:
  Super() {
  }

  virtual ~Super() {
  }

  virtual void foo() {
  }
};

class Sub: public Super {
public:
  Sub() {
  }

  virtual ~Sub() {
  }

  virtual void foo() {
  }
};

int main(void) {
  __asm__("mov $0xfffffffffffffff0, %rsp\n");
  long unsigned int *x = 0x4943E0;
  *x = 77777;

  void *foo = (void*)99;
  Super *h = new (foo) Sub();

  h->foo();

  return 0;
}
