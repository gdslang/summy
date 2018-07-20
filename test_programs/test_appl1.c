void f(int *p) {
  *p = 10;
}

int main(int argc, char **argv) {
  int x = 1;
  int y = 2;
  //int *p = &y;
  int *p;
  if(argc > 3)
    p = &x;
  else
    p = &y;
  f(p);
  return *p;
}
