#include <stdlib.h>

int g(int (**fpp)());
int f();

int main(void) {
  int (**fpp)() = malloc(sizeof(&f));
  *fpp = &f;
  int x = g(fpp);
  return x;
}

int g(int (**fpp)()) {
  return (**fpp)();
}

int f() {
  return 42;
}
