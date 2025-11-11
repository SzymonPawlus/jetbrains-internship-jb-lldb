#pragma once
#include "elf.h"
#include <optional>
#include <string>
#include <sys/types.h>

class Process {
  pid_t pid;
  ELF executable;
  std::vector<std::string> args;
  bool running;
  uintptr_t base_address;

public:
  Process(pid_t pid, const ELF &executable,
          const std::vector<std::string> &&args);
  pid_t get_pid() const;
  void spawn();
  void continue_execution();
  long read_memory(uintptr_t addr);
  void write_memory(uintptr_t addr, long value);
  void kill();
  void set_watchpoint(uintptr_t addr, int length, bool write_only);
  void wait();

  ~Process();

private:
  std::optional<uintptr_t> get_base_address();
  uintptr_t calculate_address(uintptr_t addr);
};
