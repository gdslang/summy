#include <iostream>

using namespace std;

struct A { virtual void f() = 0; };

struct B : public A {
  void f() { cout << "foo" << endl; }
};

A *g() {
  void *addr;

  __asm volatile ( "mov %%r11, %0\n"
    : "=a" (addr)
    : "a" (addr)
    : );

  B *b = new (addr) B();
  return b;
}

int main(void) {
  A *b = g();
  b->f();
  return 0;
}
