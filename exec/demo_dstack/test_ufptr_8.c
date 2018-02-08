#include <stdlib.h>

int f() {
  return 42;
}

int h(int (*fp)(void)) {
  return fp();
}

int main() {
  int x = h(&f) + h(&f);

  __asm volatile("movl %0, %%r11d" : "=a"(x) : "a"(x) : "r11");
}
