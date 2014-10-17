int main(int argc, char **argv) {
  register int a = argc;
  register int (*f)() = 0;
  if(a > 22)
    f += 4 + a;
  else
    f += 7 - a;
  return f();
}
