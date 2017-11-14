#include <iostream>
#include <map>
#include <algorithm>

using namespace std;

int main(int argc, char **argv) {
  map<int, int> a;
  map<int, int> b;
  
  a[0] = 3;
  a[1] = 4;
  
  b[0] = 6;
  b[2] = 5;
      
  map<int, int> c;
  
  set_intersection(a.begin(), a.end(), b.begin(), b.end(), inserter(c, c.begin()));
  
  for(auto it : c) {
    cout << it.first << " -> " << it.second << endl;
  }
  
  return 0;
}

