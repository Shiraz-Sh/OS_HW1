#pragma once

#include <vector>
#include <map>
#include <utility>
#include <ostream>
#include "Commands.h"

/**
 * A table of all aliased 
 */
class AliasTable{
private:
    std::map<std::string, const char*> aliases;     // store aliases and their command
    std::vector<std::string> aliases_order;         // store the aliases by order they where joined

    AliasTable() = default;

    static bool init_flag;
    static const std::vector<std::string> forbidden;
public:
    ~AliasTable() = default;

    AliasTable(AliasTable const&) = delete;     // disable copy ctor

    void operator=(AliasTable const&) = delete; // disable = operator

    static AliasTable& getInstance(){
        static AliasTable instance; // Guaranteed to be destroyed.

        // Instantiated on first use.
        if (!init_flag){
            init_flag = true;
        }
        return instance;
    }

    /**
     * Add a new alias to the table
     * @param name the new synonym for the command
     * @param command the command to alias to
     * @return true if successful
     */
    bool alias(std::string name, const char* command);

    /**
     * Removes an alias form the alias table
     * @param name the name of the synonym to un-alias
     * @return true if successful
     */
    bool unalias(std::string name);

    /**
     * Checks if an alias with a given name exists
     * @param name the alias name
     * @return if there exists an alias: `true, command`. else: `false, ""`
     */
    std::pair<bool, const char*> query(std::string name);

    /**
     * Overrides the << operator.
     * format for each alias: `<name>=<command>`
     */
    friend std::ostream& operator<<(std::ostream& os, const AliasTable& t);
};