#include "elf.h"
#include <iostream>

int main() {
  ELF elf;
  try {
    elf.load("gwatch_test");
    elf.validate();
    std::cout << "ELF file loaded and validated successfully." << std::endl;
    auto symbol = elf.get_symbol("a");
    if (symbol) {
      std::cout << "Symbol 'oragnutan' found at address: 0x" << std::hex
                << symbol->value << std::dec << std::endl;
    } else {
      std::cout << "Symbol 'oragnutan' not found." << std::endl;
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
