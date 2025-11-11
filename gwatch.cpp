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

    std::string symbol = "a";

    process.set_watchpoint(symbol, false);
    long last_value = process.read_memory(symbol);

    process.continue_execution();
    while (process.wait()) {
      long value = process.read_memory(symbol);
      if (value ==
          last_value) { // Inaccurate in some cases, but doesn't waste
                        // additional debug registers which are scarce resource
        std::cout << symbol << " " << "read" << " " << value << std::endl;
      } else {
        std::cout << symbol << " " << "write" << " " << last_value << " -> "
                  << value << std::endl;
        last_value = value;
      }
      process.continue_execution();
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
