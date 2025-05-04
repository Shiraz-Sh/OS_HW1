#include <iostream>
#include <algorithm>
#include "AliasTable.hpp"

bool AliasTable::init_flag = false;
const std::vector<std::string> AliasTable::forbidden = {
    // built-in commands
    "chpromp",  "showpid" ,     "pwd",
    "cd",       "jobs",         "fg",
    "quit",     "kill",         "alias",
    "unalias",  "unsetenv",     "watchproc",

    // special commands
    "du",       "whoami",       "netinfo",

    // forbidden symbols for aliases
    // "|",        "*",            ".",
    // "&",        "/",            "\'",
    // "="
};

bool AliasTable::alias(std::string name, const char* command){
    if (query(name).first || std::find(forbidden.begin(), forbidden.end(), name) != forbidden.end()){
        std::cerr << "smash error: alias: " << name << " already exists or is a reserved command" << std::endl;
        return false;
    }

    aliases[name] = std::string(command);
    aliases_order.push_back(name);
    return true;
}

bool AliasTable::unalias(std::string name){
    if (!query(name).first){
        return false;
    }

    aliases.erase(name);
    aliases_order.erase(std::find(aliases_order.begin(), aliases_order.end(), name));
    return true;
}

std::pair<bool, std::string> AliasTable::query(std::string name){
    if (std::find(aliases_order.begin(), aliases_order.end(), name) != aliases_order.end()){
        return { true, aliases.at(name) };
    }
    return { false, "" };
}

std::ostream& operator<<(std::ostream& os, const AliasTable& t){
    for (const auto& name : t.aliases_order){
        os << name << "=\"" << t.aliases.at(name) << "\"" << std::endl;
    }
    return os;
}