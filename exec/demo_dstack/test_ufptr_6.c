#include <stdlib.h>

int f() {
  return 42;
}

int q() {
  return 99;
}

int h(int (*fp)(void), int (*fq)(void), int a) {
  int (*ff)(void);
  if(a) {
    ff = fp;
  } else {
    ff = fq;
  }
  return ff();
}

int main() {
  int x = h(&f, &q, 99);

          __asm volatile ( "movl %0, %%r11d"
          : "=a" (x)
          : "a" (x)
          : "r11");
}
