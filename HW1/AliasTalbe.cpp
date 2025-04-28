#include <iostream>
#include <algorithm>
#include "AliasTable.hpp"

bool AliasTable::init_flag = false;
const std::vector<std::string> AliasTable::forbidden = { "quit", "kill", "pwd" };

bool AliasTable::alias(std::string name, const char* command){
    if (query(name) || std::find(forbidden.begin(), forbidden.end(), name) != forbidden.end()){
        std::cout << "smash error: alias: " << name << " already exists or is a reserved command" << std::endl;
        return false;
    }

    aliases[name] = command;
    return true;
}

bool AliasTable::unalias(std::string name){
    if (!query(name)){
        std::cout << "smash error: unalias: " << name << " alias does not exist" << std::endl;
        return false;
    }

    aliases.erase(name);
    return true;
}

bool AliasTable::query(std::string name){
    return aliases.find(name) != aliases.end();
}

std::ostream& operator<<(std::ostream& os, const AliasTable& t){
    for (const auto& item : t.aliases){
        os << item.first << "=\"" << item.second << "\"" << std::endl;
    }
    return os;
}