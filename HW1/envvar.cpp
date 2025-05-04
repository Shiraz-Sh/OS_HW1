#include "envvar.hpp"
#include "global_utils.hpp"

#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <fcntl.h>
#include <algorithm>
#include <unistd.h>

static std::vector<std::string> deleted_variables;

std::string get_environ_path(){
    pid_t pid = getpid();
    std::ostringstream oss;
    oss << "/proc/" << pid << "/environ";
    return oss.str();
}

extern char** __environ;

bool remove_environment_variable(std::string varname){
    for (size_t i = 0; __environ[i]; i++){
        if (std::strncmp(__environ[i], varname.c_str(), varname.size()) == 0){

            // Shift all remaining environment pointers one left
            for (; __environ[i]; i++)
                __environ[i] = __environ[i + 1];
            deleted_variables.push_back(varname);
            return true;
        }
    }
    return false;
}

std::pair<bool, std::string> check_envvar_exists(std::string varname, std::vector<char>& buffer){
    if (buffer.empty())
        return {false, std::string()};

    const std::string prefix = varname + "=";
    size_t i = 0;

    while (i < buffer.size()){
        std::string entry(&buffer[i]);

        if (std::strncmp(entry.c_str(), prefix.c_str(), prefix.size()) == 0){
            return { true, entry.substr(prefix.size()) };
        }
        i += entry.size() + 1;
    }
    return { false, std::string() };
}

std::vector<char> read_environment_record(){
    const std::string path = get_environ_path();
    return read_file(path);
}

std::pair<bool, std::string> is_envvar(std::string varname){
    std::vector<char> buffer = read_environment_record();
    auto check_res = check_envvar_exists(varname, buffer);
    if (check_res.first && std::count(deleted_variables.begin(), deleted_variables.end(), varname) == 0){
        return { true, check_res.second  };
    }
    return {false, ""};
}
