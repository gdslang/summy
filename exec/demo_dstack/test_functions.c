int g() {
  return 42;
}

int h() {
  return 99;
}

typedef int (*f_t)();

f_t __attribute__ ((noinline)) f(char x) {
     __asm (
     "mov    $0x4004f6,%%rax\n"
         : 
         : );
//  return h;
}

int main(int argc, char **argv) {
  int (*fp)();
  fp = f(argc);

  return fp();
}
