int g() {
  return 42;
}

int h() {
  return 99;
}

typedef int (*f_t)();

f_t __attribute__ ((noinline)) f(char x) {
//     __asm (
//     "mov    $0x4004f6,%%rax\n"
//         : 
//         : );
  if(x)
    return h;
  else
    return g;
}

int main(int argc, char **argv) {
  int (*fp)();
  fp = f(argc);

  long long x = fp();

  __asm volatile ( "mov %0, %%r11\n"
    : "=a" (x)
    : "a" (x)
    : "r11");

  return x;
}
