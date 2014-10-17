int main(int argc, char **argv) {
  register int a = argc;
  register int (*f)() = 0;
  if(a == 1)
    f += a + 3;
  else
    f += a - 7;
  if(a == 20)
    a++;
  else
    a--;
  return f();
}
