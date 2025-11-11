#include "process.h"
#include <fstream>
#include <limits.h>
#include <signal.h>
#include <sstream>
#include <stdexcept>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

static std::string canonicalize_path(const std::string &path) {
  char real_path[PATH_MAX];
  if (realpath(path.c_str(), real_path) == nullptr) {
    throw std::runtime_error("Failed to canonicalize path: " + path);
  }
  return std::string(real_path);
}

Process::Process(pid_t pid, const ELF &executable,
                 const std::vector<std::string> &&args)
    : pid(pid), executable(executable), args(args), running(false) {}

pid_t Process::get_pid() const { return pid; }
void Process::spawn() {
  // Implementation of spawning the process
  pid = fork();
  if (pid == 0) {
    // Child process
    ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
    execv(executable.get_path().c_str(),
          reinterpret_cast<char *const *>(args.data()));
    std::vector<char *> argv;
    for (const auto &arg : args) {
      argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);
    execv(executable.get_path().c_str(), argv.data());

  } else if (pid > 0) {
    running = true;
    int status;
    waitpid(pid, &status, 0); // Wait for initial stop
    base_address = get_base_address().value_or(0);
    if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
      return;
    } else {
      throw std::runtime_error("Child process did not stop as expected");
    }

  } else {
    throw std::runtime_error("Failed to fork process");
  }
}

void Process::continue_execution() {
  if (!running) {
    throw std::runtime_error("Process is not running");
  }
  ptrace(PTRACE_CONT, pid, nullptr, nullptr);
}

long Process::read_memory(uintptr_t addr) {
  errno = 0;
  long data = ptrace(PTRACE_PEEKDATA, pid, calculate_address(addr), nullptr);
  if (errno != 0) {
    throw std::runtime_error("Failed to read memory");
  }
  // Take alignment into account if needed
  uintptr_t aligment = reinterpret_cast<uintptr_t>(addr) % sizeof(long);
  data >>= (aligment * 8);

  return data;
}

void Process::write_memory(uintptr_t addr, long value) {
  if (ptrace(PTRACE_POKEDATA, pid, calculate_address(addr), value) == -1) {
    throw std::runtime_error("Failed to write memory");
  }
}

void Process::kill() {
  if (running) {
    ptrace(PTRACE_KILL, pid, nullptr, nullptr);
    waitpid(pid, nullptr, 0); // Wait for child to terminate
    running = false;
  }
}

Process::~Process() {
  if (running) {
    kill();
    running = false;
  }
}

void Process::set_watchpoint(uintptr_t addr, int length, bool write_only) {
  long dr7 =
      ptrace(PTRACE_PEEKUSER, pid, offsetof(user, u_debugreg[7]), nullptr);
  dr7 |= 1; // Enable local breakpoint 0

  int rw = write_only ? 1 : 3; // 1 for write, 3 for read/write
  int len_bits;
  switch (length) {
  case 1:
    len_bits = 0;
    break;
  case 2:
    len_bits = 1;
    break;
  case 4:
    len_bits = 3;
    break;
  case 8:
    len_bits = 2;
    break;
  default:
    throw std::invalid_argument("Invalid length for watchpoint");
  }
  // Clear RW + LEN bits for breakpoint 0
  dr7 &= ~((0b11 << 16) | (0b11 << 18)); // clear RW0+LEN0
  dr7 |= (rw << 16) | (len_bits << 18);

  ptrace(PTRACE_POKEUSER, pid, offsetof(user, u_debugreg[0]),
         calculate_address(addr));
  ptrace(PTRACE_POKEUSER, pid, offsetof(user, u_debugreg[7]), dr7);
}

void Process::wait() {
  if (!running) {
    throw std::runtime_error("Process is not running");
  }
  int status;
  waitpid(pid, &status, 0);
  if (WIFSTOPPED(status)) {
    int sig = WSTOPSIG(status);
    if (sig == SIGTRAP) {
      // This is nice, return
      return;
    }
    throw std::runtime_error("Process stopped with signal: " +
                             std::to_string(sig));
  }
}

std::optional<uintptr_t> Process::get_base_address() {
  std::string maps_path = "/proc/" + std::to_string(pid) + "/maps";
  std::ifstream maps_file(maps_path);
  if (!maps_file.is_open()) {
    return std::nullopt;
  }
  std::string line;
  // Get canonical path of the executable
  std::string exe_path = canonicalize_path(executable.get_path());
  while (std::getline(maps_file, line)) {
    std::istringstream iss(line);
    std::string address_range, perms, offset, dev, inode, pathname;
    if (!(iss >> address_range >> perms >> offset >> dev >> inode)) {
      continue;
    }
    std::getline(iss, pathname); // Get the rest of the line as pathname
    pathname.erase(0, pathname.find_first_not_of(" \t"));
    if (canonicalize_path(pathname) == exe_path) {
      size_t dash_pos = address_range.find('-');
      if (dash_pos != std::string::npos) {
        std::string start_addr_str = address_range.substr(0, dash_pos);
        uintptr_t start_addr = std::stoull(start_addr_str, nullptr, 16);
        return start_addr;
      }
    }
  }
  return std::nullopt;
}

uintptr_t Process::calculate_address(uintptr_t addr) {
  if (executable.is_pie()) {
    return base_address + addr;
  } else {
    return addr;
  }
}
