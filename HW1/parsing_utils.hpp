#pragma once

#include <string>

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

std::string _ltrim(const std::string& s);

std::string _rtrim(const std::string& s);

std::string _trim(const std::string& s);

int _parseCommandLine(const std::string& cmd_line, char** args, bool simple = true);

bool _isBackgroundComamnd(const std::string& cmd_line);

void _removeBackgroundSign(std::string& cmd_line);
