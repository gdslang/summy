struct A { virtual long f() = 0; };

struct B : public A {
  long f() { return 42; }
};

A *p;

void g() {
  p = new B();
}

int main(void) {
  g();
  long x = p->f();
  
  __asm volatile ( "movq %0, %%r11\n"
    : "=a" (x)
    : "a" (x)
    : "r11");

  return 0;
}
