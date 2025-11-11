#include <iostream>

int a = 0;
int b = 0;
int c = 1;

int main() {
  for (int i = 0; i < 30; ++i) {
    a = b;
    b = c;
    c = a + b;
  }
  std::cout << "Fibonacci sequence computed up to 50: " << c << std::endl;
}
