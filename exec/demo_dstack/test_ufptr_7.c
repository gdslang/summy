#include <stdlib.h>

int f() {
  return 42;
}

int g() {
  return 11;
}

int q() {
  return 99;
}

int h(int (*fp1)(void), int (*fp2)(void)) {
  return fp1() + fp2();
}

int main(int argc, char* argv[]) {
  int (*fp1)(void);
  int (*fp2)(void);
  if(argc == 3) {
    fp1 = &f;
    fp2 = &q;
  } else if(argc == 9) {
    fp1 = &q;
    fp2 = &f;
  } else {
    // fp1 = &q;
    // fp2 = &f;

    fp1 = &g;
    fp2 = &g;
  }

  int x = h(fp1, fp2);

  __asm volatile("movl %0, %%r11d" : "=a"(x) : "a"(x) : "r11");
}
