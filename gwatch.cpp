#include "elf.h"
#include "process.h"
#include <iostream>

int main() {
  ELF elf;
  try {
    elf.load("gwatch_test");
    elf.validate();

    Process process(0, elf, {});
    process.spawn();

    std::cout << "ELF file loaded and validated successfully." << std::endl;
    auto symbol = elf.get_symbol("b");
    if (symbol) {
      process.set_watchpoint(symbol->value, 4, true);
      process.continue_execution();
      while (true) {
        process.wait();
        long value = process.read_memory(symbol->value);
        std::cout << "Watchpoint hit! New value of 'a': " << value << std::endl;
        process.continue_execution();
      }
    } else {
      throw std::runtime_error("Symbol not found");
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
