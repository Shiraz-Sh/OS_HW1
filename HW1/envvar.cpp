#include "envvar.hpp"

#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <bits/stdc++.h>

static std::vector<std::string> deleted_variables;

std::string get_environ_path(){
    pid_t pid = getpid();
    std::ostringstream oss;
    oss << "/proc/" << pid << "/environ";
    return oss.str();
}


bool remove_environment_variable(std::string varname){
    for (size_t i = 0; environ[i]; i++){
        if (std::strncmp(environ[i], varname.c_str(), varname.size()) == 0){

            // Shift all remaining environment pointers one left
            for (; environ[i]; i++)
                environ[i] = environ[i + 1];
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
    std::vector<char> buffer;

    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1){
        perror("open failed");
        return buffer;
    }

    const size_t chunk_size = 4096; // I think it is a reasonable size for a chunk

    while (true){
        size_t current_size = buffer.size();
        buffer.resize(current_size + chunk_size);
        ssize_t nread = read(fd, buffer.data() + current_size, chunk_size);

        // EOF encountered
        if (nread == 0)
            break;
        if (nread < 0){
            perror("read failed");
            buffer.clear();
            break;
        }

        buffer.resize(current_size + nread);
        if (nread < static_cast<ssize_t>(chunk_size)) break; // finished reading
    }

    close(fd);
    return buffer;
}

std::pair<bool, std::string> is_envvar(std::string varname){
    std::vector<char> buffer = read_environment_record();
    auto check_res = check_envvar_exists(varname, buffer);
    if (check_res.first && std::count(deleted_variables.begin(), deleted_variables.end(), varname) == 0){
        return { true, check_res.second  };
    }
    return {false, ""};
}
