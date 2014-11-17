int main(int argc, char **argv) {
  register int a = argc;
  register int (**f)();// = 0;
	*f = 42;
  x:
  if(a == 1) {
    if(a == 2)
      a++;
    else
      *f += 4;
    if(a == 3)
      goto x;
  } else
//    f += 7 - a;
		a++;
  return (*f)();
}
