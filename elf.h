#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>

enum class ELFSize : uint8_t { ELF32 = 1, ELF64 = 2 };
enum class ELFEndianness : uint8_t { Little = 1, Big = 2 };
enum class ELFType : uint16_t {
  None = 0,
  Relocatable = 1,
  Executable = 2,
  Shared = 3,
  Core = 4
};

enum class ELFInstructionSet : uint16_t {
  None = 0,
  x86 = 3,
  ARM = 40,
  SuperH = 42,
  IA_64 = 50,
  x86_64 = 62,
  AArch64 = 183,
};

struct elf64_header_t {
  uint8_t magic[4];
  uint8_t size;       // 1 - 32bit, 2 - 64bit
  uint8_t endianness; // 1 - little, 2 - big
  uint8_t version;
  uint8_t os_abi;
  uint8_t padding[8];
  uint16_t type;
  uint16_t machine;
  uint32_t version2;
  uint64_t entry; // entry point
  uint64_t phoff; // program header offset
  uint64_t shoff; // section header offset
  uint32_t flags;
  uint16_t ehsize;
  uint16_t phentsize;
  uint16_t phnum;
  uint16_t shentsize;
  uint16_t shnum;
  uint16_t shstrndx;
};

// Program header
struct elf64_phdr_t {
  uint32_t type;
  uint32_t flags;
  uint64_t offset;
  uint64_t vaddr;
  uint64_t paddr;
  uint64_t filesz;
  uint64_t memsz;
  uint64_t align;
};

// Section header
struct elf64_shdr_t {
  uint32_t name;
  uint32_t type;
  uint64_t flags;
  uint64_t addr;
  uint64_t offset;
  uint64_t size;
  uint32_t link;
  uint32_t info;
  uint64_t addralign;
  uint64_t entsize;
};

struct elf64_sym_t {
  uint32_t name;
  uint8_t info;
  uint8_t other;
  uint16_t shndx;
  uint64_t value;
  uint64_t size;
};

class ELF {
  std::vector<uint8_t> data;
  std::string path_;
  std::map<std::string, elf64_shdr_t *> section_cache;
  elf64_header_t *get_header();
  elf64_phdr_t *get_program_header(size_t index);
  elf64_shdr_t *get_section_header(size_t index);
  elf64_shdr_t *get_section_header(const std::string &name);
  elf64_sym_t *get_symbol_from_table(elf64_shdr_t *symtab_header,
                                     elf64_shdr_t *strtab_header,
                                     const std::string &name);

public:
  ELF();
  void load(const std::string &path);
  void validate() const;
  ELFSize getSize() const;
  elf64_sym_t *get_symbol(const std::string &name);
  const std::string &get_path() const;
  bool is_pie();
};
