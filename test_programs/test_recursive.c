#include <stdio.h>

int rec(unsigned int n) {
  if(n > 0)
    return rec(n - 1);
  else
    return n;
}

int main(void) {
  return rec(42);
}
