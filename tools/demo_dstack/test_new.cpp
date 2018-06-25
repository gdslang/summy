#include <iostream>

using namespace std;

class Hugo {
  private: int x;

  public: Hugo() {
    x = 99;
  }
};

int main(void) {
  Hugo *h = new Hugo();
  return 0;
}
