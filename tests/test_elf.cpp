#include "../elf.h"
#include <gtest/gtest.h>

TEST(ELFTest, ValidateCorrectELF) {
  ELF elf;
  ASSERT_NO_THROW(elf.load("tested_programs/basic_test"));
  ASSERT_NO_THROW(elf.validate());
}

TEST(ELFTest, ValidateIncorrectELF) {
  ELF elf;
  ASSERT_NO_THROW(elf.load("tested_programs/invalid_file"));
  ASSERT_THROW(elf.validate(), std::runtime_error);
}

TEST(ELFTest, LoadNonExistentFile) {
  ELF elf;
  ASSERT_THROW(elf.load("non_existent_file"), std::runtime_error);
}

TEST(ELFTest, GetExistingSymbol) {
  ELF elf;
  elf.load("tested_programs/basic_test");
  elf.validate();
  elf64_sym_t *symbol = elf.get_symbol("a");
  ASSERT_NE(symbol, nullptr);
  EXPECT_EQ(symbol->size, sizeof(int));
}

TEST(ELFTest, GetNonExistingSymbol) {
  ELF elf;
  elf.load("tested_programs/basic_test");
  elf.validate();
  elf64_sym_t *symbol = elf.get_symbol("non_existent_symbol");
  EXPECT_EQ(symbol, nullptr);
}

TEST(ELFTest, CheckPIE) {
  ELF elf;
  elf.load("tested_programs/basic_test");
  elf.validate();
  EXPECT_TRUE(elf.is_pie());

  ELF elf2;
  elf2.load("tested_programs/basic_no_pie_test");
  elf2.validate();
  EXPECT_FALSE(elf2.is_pie());
}
