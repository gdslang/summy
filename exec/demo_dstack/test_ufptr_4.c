#include <stdlib.h>

int f() {
	return 42;
}

int q() {
	return 99;
}

int h(int (*fp)(void)) {
  return fp();
}

int g(int (*fp)(int(*)(void))) {
	return fp(&f);
}

int main() {
	int x = g(&h) + h(&q);

          __asm volatile ( "movl %0, %%r11d"
          : "=a" (x)
          : "a" (x)
          : "r11");
}
