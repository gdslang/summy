#include <stdio.h>

__attribute__((noinline))
static int fab() {
	return 42;
}

int foo(int z) {
  int x = 7 - z;
  return x + z;
}

int main() {
	return fab();
}
