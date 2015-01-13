int main(int argc, char **argv) {
  register long int a = 3;
  register int (*f)() = 10;
  if(a > 22)
    f += 4 + a;
  else
    f += 5 - a;
  return f();
}
