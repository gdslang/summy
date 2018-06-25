#include <stdio.h>
#include <stdlib.h>
#include <string>

struct hugo {
  virtual int foo() {
    return 99;
  }
  virtual ~hugo() { }
};

struct inge : public hugo {
  virtual int foo() {
    return 42;
  }
  virtual ~inge() { }
};

long long g(hugo *h) {
  long long x = h->foo();
  return x;
}

int main(int argc, char **argv) {
  void *addr;

  __asm volatile ( "mov %%r11, %0\n"
    : "=a" (addr)
    : "a" (addr)
    : );

  hugo *h = new (addr) inge();
  //hugo *h = new (addr) hugo();

  __asm volatile ( "jmp after_new\n" );
  __asm volatile ( "after_new:\n" );

  long long x = g(h);
  //long long x = h->foo();

  __asm volatile ( "mov %0, %%r11\n"
    : "=a" (x)
    : "a" (x)
    : "r11");

  return x;
}
