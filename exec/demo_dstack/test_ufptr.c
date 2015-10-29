#include <stdlib.h>

int f() {
	return 42;
}

int g(int (*fp)(void)) {
	return fp();
}

int main() {
	return g(&f);
}
