int main(int argc, char **argv) {
  register int a = 13;
  register int (**f)();
  *f = 42;
  if(a == 1) {
	  if(a == 3)
		  a++;
    if(a == 2) {
			f += a + 1;
      a = 5;
    } else if(a == 9)
		  a++;
	  else
      *f += 4;
  } else
    *f += 7 - a;
//{((419 -> 420) -> {36, 42, 46})}
  if(argc > 3) {
    a = 2000 - 3*a;
    *f = a;
  } else
    *f += 4000 + a;
  return (*f)();
}
//{((599 -> 600) -> {1955, 1958, 1961, 1985, 4049, 4056, 4057, 4059, 4060})}
