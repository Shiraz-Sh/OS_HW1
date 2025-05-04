#pragma once

#include <string>
#include <vector>

#define SYSCALL_FAIL(name) perror(std::string("smash error: " + std::string(name) + " failed").c_str())
#define KB 1024

std::vector<char> read_file(const std::string& path);

std::vector<std::string> list_directory(const std::string& path);
