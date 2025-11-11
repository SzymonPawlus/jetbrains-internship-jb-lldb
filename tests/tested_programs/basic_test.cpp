#include <iostream>

int a = 5;
int b = 10;
int c = 0;
int64_t large_var = 0x1234567890ABCDEF;
struct {
  int x;
  double y;
  float z;
} unused_struct;
int main() {
  // Make a loop of fibbonaci-like iterations
  for (int i = 0; i < 30; ++i) {
    int temp = a;
    a = b;
    b = c;
    c = temp + b;
    std::cout << "Iteration " << i << ": a=" << a << ", b=" << b << ", c=" << c
              << std::endl;
  }
}
