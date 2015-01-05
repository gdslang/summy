int main(int argc, char **argv) {
  register int a = 13;
  register int (**f)();
  *f = 42;
  if(a == 1) {
    if(a == 2)
      a++;
    else
      *f += 4;
  } else
    f += 7 - a;
//  if(argc > 3) {
//    a = 2000 - 3*a;
//    f = a;
//  } else
//    f += 11;
  return (*f)();
}
