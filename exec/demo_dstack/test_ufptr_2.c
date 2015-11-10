#include <stdlib.h>

int f() {
	return 42;
}

int h(int (*fp)(void)) {
  return fp();
}

int g(int (*fp)(void)) {
	return h(fp);
}

int main() {
	return g(&f);
}
