#include <iostream>

int64_t a = 0;
int64_t b = 0;
int64_t c = 1;

int main() {
  for (int i = 0; i < 50; ++i) {
    a = b;
    b = c;
    c = a + b;
  }
  std::cout << "Fibonacci sequence computed up to 50: " << c << std::endl;
}
