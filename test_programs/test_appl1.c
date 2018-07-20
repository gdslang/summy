void f(int *p, int *q) {
  *p += 10;
  *q = *p;
}

int main(int argc, char **argv) {
  int x = 1;
  int y = 2;
  //int *p = &y;
  int *p;
  if(argc > 3)
    p = &x;
  else
    p = &y;
  int q;
  f(p, &q);

  __asm volatile (
    "mov %0, %%r11d\n"\
    "mov %1, %%r12d\n"\
    "mov %2, %%r13d"
    : "=r" (x), "=r" (y), "=r" (q)
    : "r" (x), "r" (y), "r" (q)
    : "r11", "r12", "r13");

  return 0;
}
