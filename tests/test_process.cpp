#include "../elf.h"
#include "../process.h"
#include <gtest/gtest.h>

TEST(ProcessTest, SpawnProcess) {
  ELF elf;
  elf.load("tested_programs/basic_test");
  elf.validate();

  Process process(elf, {});
  ASSERT_NO_THROW(process.spawn());
}

TEST(ProcessTest, ReadMemory) {
  ELF elf;
  elf.load("tested_programs/basic_test");
  elf.validate();

  elf64_sym_t *symbol = elf.get_symbol("b");
  ASSERT_NE(symbol, nullptr);
  EXPECT_EQ(symbol->size, sizeof(int));

  Process process(elf, {});
  process.spawn();
  long b_value;
  ASSERT_NO_THROW(b_value = process.read_memory("b"));

  EXPECT_EQ(b_value, 10);
  process.kill();
}

TEST(ProcessTest, ReadMemoryNoPie) {
  ELF elf;
  elf.load("tested_programs/basic_no_pie_test");
  elf.validate();

  elf64_sym_t *symbol = elf.get_symbol("b");
  ASSERT_NE(symbol, nullptr);
  EXPECT_EQ(symbol->size, sizeof(int));

  Process process(elf, {});
  process.spawn();
  long b_value;
  ASSERT_NO_THROW(b_value = process.read_memory("b"));

  EXPECT_EQ(b_value, 10);
  process.kill();
}

TEST(ProcessTest, SetWatchpoint) {
  ELF elf;
  elf.load("tested_programs/basic_test");
  elf.validate();

  Process process(elf, {});
  process.spawn();
  ASSERT_NO_THROW(process.set_watchpoint("a", false));
  process.kill();
}

TEST(ProcessTest, StopOnWatchpoint) {
  ELF elf;
  elf.load("tested_programs/basic_test");
  elf.validate();

  Process process(elf, {});
  process.spawn();
  process.set_watchpoint("a", false);

  long last_value = process.read_memory("a");
  bool hit_watchpoint = false;

  for (int i = 0; i < 30; ++i) {
    process.continue_execution();
    ASSERT_NO_THROW({
      if (!process.wait()) {
        break; // Process exited
      }
    });

    long current_value = process.read_memory("a");
    if (current_value != last_value) {
      hit_watchpoint = true;
      break;
    }
    last_value = current_value;
  }

  EXPECT_TRUE(hit_watchpoint);
  process.kill();
}

TEST(ProcessTest, InvalidSymbolWatchpoint) {
  ELF elf;
  elf.load("tested_programs/basic_test");
  elf.validate();

  Process process(elf, {});
  process.spawn();
  EXPECT_THROW(process.set_watchpoint("non_existent_symbol", false),
               std::runtime_error);
  process.kill();
}

TEST(ProcessTest, InvalidSymbolSize) {
  ELF elf;
  elf.load("tested_programs/basic_test");
  elf.validate();

  Process process(elf, {});
  process.spawn();
  EXPECT_THROW(process.set_watchpoint("unused_struct", false),
               std::invalid_argument);
  process.kill();
}
