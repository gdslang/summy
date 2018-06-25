int g() {
  return 42;
}

int h() {
  return 99;
}

int __attribute__ ((noinline)) f(int (**fp)(), char x) {
//  if(x > 2)
//    *fp = g;
//  else
    *fp = h;
}

int main(int argc, char **argv) {
  int (*fp)();

  f(&fp, argc);

  return fp();
}
