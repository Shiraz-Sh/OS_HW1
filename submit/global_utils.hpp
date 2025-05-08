#pragma once

#include <string>
#include <vector>

#define SYSCALL_FAIL(name) perror(std::string("smash error: " + std::string(name) + " failed").c_str())
#define KB 1024
#define FORK_NOTIFY(pid, code) { SmallShell::getInstance().set_fg_pid(pid); { code }; SmallShell::getInstance().reset_fg_pid(); }

std::vector<char> read_file(const std::string& path);

std::vector<std::string> list_directory(const std::string& path);
