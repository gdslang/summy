int main(void) {
  register int a = 3;
  register int (*f)() = 15;
  x:
  if(a == 1) {
    if(a == 2)
      a++;
    else
      f += 4 + a;
  } else
    f += 7 - a;
  return f();
}
