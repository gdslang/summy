#include <stdlib.h>

int main(int argc, char **argv) {
  size_t *x;
  for(int i = 0; i < argc; i++) {
    x = (int*)*x;
  }
  return (int)*x;
}
