#pragma once

#include <string>

#define SYSCALL_FAIL(name) perror(std::string("smash error: " + std::string(name) + " failed").c_str())