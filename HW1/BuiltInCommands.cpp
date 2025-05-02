
#include <bits/stdc++.h>
#include <unistd.h>
#include <regex>
#include <cstddef>
#include <sys/wait.h>
#include <fcntl.h>     // open
#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include "BuiltInCommands.h"
#include "JobsList.h"
#include "AliasTable.hpp"

#define MAX_PATH 200

using namespace std;

void ChpromptCommand::execute() {
    this->prepare();
    if (count >= 2) {
        SmallShell::getInstance().setChprompt(strcat(args[1], "> "));
    } else { // no parameters are provided
        SmallShell::getInstance().setChprompt("smash> ");
    }
    this->cleanup();
}

void ShowpidCommand::execute() {
    this->prepare();
    pid_t pid = getpid();
    if (pid == -1) {
        // If getpid() fails, handle the error
        perror("smash error: getpid failed");
    } else {
        std::cout << "smash pid is " << pid << std::endl;
    }
    this->cleanup();
}

void PwdCommand::execute() {
    this->prepare();
    char* cwd = getcwd(nullptr, 0);  // let getcwd allocate enough memory
    if (cwd == nullptr) {
        perror("smash error: getcwd failed");
        this->cleanup();
        return;
    }
    std::cout << cwd << std::endl;
    free(cwd);
    this->cleanup();
}

void CdCommand::execute() {
    this->prepare();
    if (count > 2) {  // If more than one argument was provided
        std::cerr << "smash error: cd: too many arguments" << std::endl;
        this->cleanup();
        return;
    }

    SmallShell& smash = SmallShell::getInstance();

    if (*args[1] == '-') {
        if (smash.oldPWD == nullptr) {  // If the last working directory is empty and `cd -` was called
            std::cerr << "smash error: cd: OLDPWD not set" << std::endl;
            this->cleanup();
            return;
        } else { // Else change to last working directory
            char* temp = strdup(smash.oldPWD); // make a full copy
            if (!savingLastWorkingDict()) {
                free(temp);
                return;
            }
            if (chdir(temp) != 0) {
                perror("smash error: chdir failed");
            }
            free(temp);
        }
    } else if (args[1] != nullptr) {
        // Saving last working directory
        if (!savingLastWorkingDict()) {
            return;
        }
        // Change to given directory
        if (chdir(args[1]) != 0) {
            perror("smash error: chdir failed");
        }
    }
    this->cleanup();
}

bool CdCommand::savingLastWorkingDict() {
    SmallShell& smash = SmallShell::getInstance();
    if (smash.oldPWD != nullptr) {
        free(smash.oldPWD);
    }
    smash.oldPWD = getcwd(nullptr, 0);
    if (smash.oldPWD == nullptr) {
        perror("smash error: getcwd failed");
        this->cleanup();
        return false;
    }
    return true;
}

/**
* cheak if a specific string is a positive number
* @param s
* @return
*/
bool isNumber(const char* s) {
    if (!s || *s == '\0') return false;
    for (int i = 0; s[i]; ++i) {
        if (!std::isdigit(s[i])) return false;
    }
    if (*s == '0' || s[0] == '-') return false;
    return true;
}

void FgCommand::execute() {
    this->prepare();
    if (count > 2 ||!isNumber(args[1])) {  // If more than one argument was provided / the format of the arguments isn't correct
        std::cerr << "smash error: fg: invalid arguments" << std::endl;
        this->cleanup();
        return;
    }

    SmallShell& smash = SmallShell::getInstance();
    if (args[1] != nullptr) {
        int givenGobID = std::stoi(args[1]);
        if (smash.jobs_list.getJobById(givenGobID) == nullptr) { // If job-id was specified with a job id that does not exist
            std::cerr << "smash error: fg: job-id " << givenGobID << " does not exist" << std::endl;
            this->cleanup();
            return;
        }
        // brings the givenJobID to foreground
        bringJobToForeground(givenGobID);
    } else { // If no other argument was given
        if (smash.jobs_list.isJobsListEmpty()) {
            std::cerr << "smash error: fg: jobs list is empty" << std::endl;
            this->cleanup();
            return;
        }
        // brings the job with the maximal job id in the jobs list to foregrund
        bringJobToForeground(smash.jobs_list.getMaxJobID()->getJobID());
    }
    this->cleanup();
}

void FgCommand::bringJobToForeground(int jobID) {
    SmallShell& smash = SmallShell::getInstance();
    // smash waits for certain job to finish, meaning it will be brought to foreground
    pid_t wpid = waitpid(smash.jobs_list.getJobById(jobID)->getJobPid(), smash.jobs_list.getJobById(jobID)->getWstatus(), 0);
    if (wpid == -1) {
        perror("smash error: waitpid failed");
        return;
    }
}

void QuitCommand::execute(){
    prepare();

    // if optional argument kill is given
    if (count == 2 && strcmp(args[1], "kill") == 0){
        JobsList::getInstance().killAllJobs();
    }

    cleanup();

    exit(0);
}

void AliasCommand::execute(){
    prepare();

    AliasTable& table = AliasTable::getInstance();

    // For one argument just print all aliases
    if (count == 1){
        std::cout << table;
        return;
    }

    if (count != 2){
        // TODO: if more than one argument provided???
    }

    // Check if input is valid
    std::regex pattern("(^([a-zA-Z0-9_]+)='([^']*)'$)");
    std::smatch matches;
    std::string str(args[1]);

    if (!std::regex_match(str, matches, pattern)){
        std::cout << "smash error: alias: invalid alias format" << std::endl;
        return;
    }

    std::string name = matches[1];
    std::string command = matches[2];
    table.alias(name, command.c_str());

    cleanup();
}

void WatchprocCommand::execute() {
    if (count > 2 ||!isNumber(args[1])) {  // If more than one argument was provided / the format of the arguments isn't correct
        std::cerr << "smash error: watchproc: invalid arguments" << std::endl;
        this->cleanup();
        return;
    }
    // std::cout << "PID: " << args[1] << " | CPU Usage: " << cpuUsage << " | Memory Usage: " << memUsage << std::endl;
}

// ------------------------------------------------------------------ Utility funcitons for UnSetEnv -------------------------------------------------------------------
/**
 * Returns the path to the current environment variables state file
 */
std::string get_environ_path(){
    pid_t pid = getpid();
    std::ostringstream oss;
    oss << "/proc/" << pid << "/environ";
    return oss.str();
}


/**
 * Remove an environment variable
 * @param varname the variable to remove
 * @return if removal succeeded
 */
bool remove_environment_variable(std::string varname){
    for (size_t i = 0; environ[i]; i++){
        if (std::strncmp(environ[i], varname.c_str(), varname.size()) == 0){

            // Shift all remaining environment pointers one left
            for (; environ[i]; i++)
                environ[i] = environ[i + 1];
            return true;
        }
    }
    return false;
}

/**
 * Check if the variable is in the environment
 * @param varname the name of the variable to search for
 * @param buffer the state of the environment variables file parsed to chars
 * @returns true if the variable was found
 */
bool check_envvar_exists(std::string varname, std::vector<char>& buffer){
    if (buffer.empty())
        return false;

    const std::string prefix = varname + "=";
    size_t i = 0;

    while (i < buffer.size()){
        std::string entry(&buffer[i]);
        std::cout << entry << std::endl;

        if (std::strncmp(entry.c_str(), prefix.c_str(), prefix.size()) == 0){
            return true;
        }
        i += entry.size() + 1;
    }
    return false;
}

/**
 * Reads the entire environment records into a char buffer for later parsing
 * @returns the file parsed to chars
 */
std::vector<char> read_environment_record(){
    const std::string path = get_environ_path();
    std::vector<char> buffer;

    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1){
        std::cerr << "Could not open open the file: " << path << std::endl;
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
            std::cerr << "read error from " << path << std::endl;
            buffer.clear();
            break;
        }

        buffer.resize(current_size + nread);
        if (nread < static_cast<ssize_t>(chunk_size)) break; // finished reading
    }

    close(fd);
    return buffer;
}

// --------------------------------------------------------------- End of utility funcitons for UnSetEnv ---------------------------------------------------------------
void UnSetEnvCommand::execute(){
    prepare();

    static std::vector<std::string> deleted_variables;

    if (count == 1){
        std::cout << "smash error: unsetenv: not enough arguments" << std::endl;
    }

    for (int i = 1; i < count; i++){
        std::string varname(args[i]);

        // we need to read it every time beacuse it updates after removal
        std::vector<char> buffer = read_environment_record();

        // check the variable exists
        if (!check_envvar_exists(varname, buffer) || std::count(deleted_variables.begin(), deleted_variables.end(), varname) != 0){
            std::cerr << "smash error: unsetenv: " << varname << " does not exist" << std::endl;
            return;
        }

        // remove the variable
        if (!remove_environment_variable(varname)){
            std::cerr << "Removal of environment variable " << varname << " failed" << std::endl; // This should not occur in any case! 
            return;
        }
        deleted_variables.push_back(varname);
    }

    cleanup();
}