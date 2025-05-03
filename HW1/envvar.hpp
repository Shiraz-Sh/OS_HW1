#pragma once

#include <string>
#include <vector>

/**
 * Returns the path to the current environment variables state file
 */
std::string get_environ_path();

/**
 * Remove an environment variable
 * @param varname the variable to remove
 * @return if removal succeeded
 */
bool remove_environment_variable(std::string varname);
/**
 * Check if the variable is in the environment
 * @param varname the name of the variable to search for
 * @param buffer the state of the environment variables file parsed to chars
 * @returns true, entry if the variable was found
 */
std::pair<bool, std::string> check_envvar_exists(std::string varname, std::vector<char>& buffer);

/**
 * Reads the entire environment records into a char buffer for later parsing
 * @returns the file parsed to chars
 */
std::vector<char> read_environment_record();


/**
 * Checks if a string is an environment variable name
 * @param varname the string to check on
 * @return true, entry if the string is an environment variable
 */
std::pair<bool, std::string> is_envvar(std::string varname);