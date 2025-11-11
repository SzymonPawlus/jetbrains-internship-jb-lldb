#pragma once
#include "elf.h"
#include <map>
#include <optional>
#include <string>
#include <sys/types.h>

enum class ContidtionType { Read, Write, ReadWrite };

class Process {
  pid_t pid;
  ELF executable;
  std::vector<std::string> args;
  bool running;
  uintptr_t base_address;
  std::map<std::string, elf64_sym_t> symbol_cache;

public:
  Process(const ELF &executable, const std::vector<std::string> &&args);

  pid_t get_pid() const;
  void spawn();
  void continue_execution();
  void kill();

  long read_memory(const std::string &symbol_name);
  // UNUSED function written for completeness
  void write_memory(const std::string &symbol_name, long value);

  void set_watchpoint(const std::string &symbol_name, bool write_only);

  // Returns true if process hasn't exited yet
  // Throws if process stopped with a signal other than SIGTRAP
  // Returns false if process has exited
  bool wait();

  ~Process();

private:
  std::optional<uintptr_t> get_base_address();
  uintptr_t calculate_address(uintptr_t addr);
  const elf64_sym_t *find_symbol(const std::string &name);
};
