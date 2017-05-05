#include <stdlib.h>

int f1() {
  return 42;
}

int g1() {
  return 99;
}

int f2() {
  return 86;
}

int g2() {
  return 0;
}

int h(int (*fp0)(void), int (*fp1)(void)) {
  return fp0() + fp1();
}

int main(int argc) {
  int (*fp0)(void);
  int (*fp1)(void);
  if(argc == 0) {
    fp0 = &f1;
    fp1 = &g1;
  } else {
    fp0 = &f2;
    fp1 = &g2;
  }
  h(fp0, fp1);
}
