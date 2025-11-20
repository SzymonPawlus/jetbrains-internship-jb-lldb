#include "elf.h"
#include <fstream>

ELF::ELF() : data(), path_(""), section_cache() {}

void ELF::load(const std::string &path) {
  path_ = path;

  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file) {
    throw std::runtime_error("Failed to open ELF file: " + path);
  }

  std::streamsize size = file.tellg();
  if (size < sizeof(elf64_header_t)) {
    throw std::runtime_error("File too small to be a valid ELF: " + path);
  }

  file.seekg(0, std::ios::beg);
  data.resize(size);
  if (!file.read(reinterpret_cast<char *>(data.data()), size)) {
    throw std::runtime_error("Failed to read ELF file: " + path);
  }
  
  // Clear cache when loading new file
  section_cache.clear();
}

void ELF::validate() const {
  const elf64_header_t *header =
      reinterpret_cast<const elf64_header_t *>(data.data());

  // Check magic number
  if (header->magic[0] != 0x7F || header->magic[1] != 'E' ||
      header->magic[2] != 'L' || header->magic[3] != 'F') {
    throw std::runtime_error("Invalid ELF magic number");
  }

  if (header->size != static_cast<uint8_t>(ELFSize::ELF64)) {
    throw std::runtime_error("Unsupported ELF size (only 64-bit supported)");
  }
  if (header->endianness != static_cast<uint8_t>(ELFEndianness::Little)) {
    throw std::runtime_error(
        "Unsupported ELF endianness (only little-endian supported)");
  }
}

elf64_header_t *ELF::get_header() {
  return reinterpret_cast<elf64_header_t *>(data.data());
}

elf64_phdr_t *ELF::get_program_header(size_t index) {
  elf64_header_t *header = get_header();
  if (index >= header->phnum) {
    throw std::out_of_range("Program header index out of range");
  }
  return reinterpret_cast<elf64_phdr_t *>(data.data() + header->phoff +
                                          index * header->phentsize);
}

elf64_shdr_t *ELF::get_section_header(size_t index) {
  elf64_header_t *header = get_header();
  if (index >= header->shnum) {
    throw std::out_of_range("Section header index out of range");
  }
  return reinterpret_cast<elf64_shdr_t *>(data.data() + header->shoff +
                                          index * header->shentsize);
}

elf64_shdr_t *ELF::get_section_header(const std::string &name) {
  // Check cache first
  auto it = section_cache.find(name);
  if (it != section_cache.end()) {
    return it->second;
  }
  
  elf64_header_t *header = get_header();
  elf64_shdr_t *shstrtab_header = get_section_header(header->shstrndx);
  const char *shstrtab =
      reinterpret_cast<const char *>(data.data() + shstrtab_header->offset);

  for (size_t i = 0; i < header->shnum; ++i) {
    elf64_shdr_t *section_header = get_section_header(i);
    const char *section_name = shstrtab + section_header->name;
    if (name == section_name) {
      // Cache the result
      section_cache[name] = section_header;
      return section_header;
    }
  }
  throw std::runtime_error("Section not found: " + name);
}

elf64_shdr_t *ELF::get_section_header_safe(const std::string &name) {
  // Check cache first
  auto it = section_cache.find(name);
  if (it != section_cache.end()) {
    return it->second;
  }
  
  elf64_header_t *header = get_header();
  elf64_shdr_t *shstrtab_header = get_section_header(header->shstrndx);
  const char *shstrtab =
      reinterpret_cast<const char *>(data.data() + shstrtab_header->offset);

  for (size_t i = 0; i < header->shnum; ++i) {
    elf64_shdr_t *section_header = get_section_header(i);
    const char *section_name = shstrtab + section_header->name;
    if (name == section_name) {
      // Cache the result
      section_cache[name] = section_header;
      return section_header;
    }
  }
  // Cache nullptr result to avoid repeated searches
  section_cache[name] = nullptr;
  return nullptr;
}

elf64_sym_t *ELF::get_symbol_from_table(elf64_shdr_t *symtab_header,
                                        elf64_shdr_t *strtab_header,
                                        const std::string &name) {
  const char *strtab =
      reinterpret_cast<const char *>(data.data() + strtab_header->offset);
  size_t num_symbols = symtab_header->size / symtab_header->entsize;

  for (size_t i = 0; i < num_symbols; ++i) {
    elf64_sym_t *symbol = reinterpret_cast<elf64_sym_t *>(
        data.data() + symtab_header->offset + i * symtab_header->entsize);
    const char *symbol_name = strtab + symbol->name;
    if (name == symbol_name) {
      return symbol;
    }
  }
  return nullptr;
}

elf64_sym_t *ELF::get_symbol(const std::string &name) {
  // Try first in .symtab
  elf64_shdr_t *symtab_header = get_section_header_safe(".symtab");
  elf64_shdr_t *strtab_header = get_section_header_safe(".strtab");
  if (symtab_header && strtab_header) {
    elf64_sym_t *symbol =
        get_symbol_from_table(symtab_header, strtab_header, name);
    if (symbol) {
      return symbol;
    }
  }
  // Then try in .dynsym
  symtab_header = get_section_header_safe(".dynsym");
  strtab_header = get_section_header_safe(".dynstr");
  if (symtab_header && strtab_header) {
    elf64_sym_t *symbol =
        get_symbol_from_table(symtab_header, strtab_header, name);
    return symbol;
  }
  return nullptr;
}

const std::string &ELF::get_path() const { return path_; }

bool ELF::is_pie() {
  elf64_header_t *header = get_header();
  return header->type == 3; // ET_DYN
}
