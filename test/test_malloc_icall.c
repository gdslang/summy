#include <stdlib.h>

int f() {
  return 42;
}

int g(int (**fpp)()) {
  return (**fpp)();
}

int main(void) {
  int (**fpp)() = malloc(sizeof(&f));
  *fpp = &f;
  int x = g(fpp);
  return x;
}
