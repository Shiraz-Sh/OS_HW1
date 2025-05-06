#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include <algorithm>
#include <sys/wait.h>
#include <iomanip>
#include <string>
#include <sys/stat.h>

#include "Commands.hpp"
#include "BuiltInCommands.hpp"
#include "JobsList.hpp"
#include "SpecialCommands.hpp"
#include "envvar.hpp"
#include "parsing_utils.hpp"

#define MAX_ARGS 20

// volatile pid_t foreground_pid = -1;

using namespace std;

void Command::prepare(){
    count = _parseCommandLine(this->cmd_line, args);
}

void Command::cleanup() {
    // free space _parseCommandLine allocated //TODO: maybe needs to be in destructor?
    for (int i = 0; i < count; ++i) {
        free(args[i]);
    }
}

/**
 * @param cmd_line
 * @return true if cmd_line contain '?' or '*'
 */
bool checkWildcards(const char* cmd_line){
    std::string command = std::string(cmd_line);
    return command.find("?") != std::string::npos || command.find("*") != std::string::npos;
}

/**
 * Checks if > is found in the command (if only one > is found return true)
 */
bool checkIORedirection(const char* cmd_line){
    return std::string(cmd_line).find(">") != std::string::npos;
}

bool checkPipe(const char* cmd_line){
    return std::string(cmd_line).find("|") != std::string::npos;
}

void complexExternalCommand::execute() {
    this->prepare();
    std::string combinedCommand;
    for (int i = 0; args[i] != nullptr; ++i) {
        combinedCommand += args[i];
        combinedCommand += " ";
    }
    combinedCommand = _trim(combinedCommand); // if you have a trim function

    char* const newArgv[] = {
            (char*)"/bin/bash",         // path to bash
            (char*)"-c",                // tells bash to execute the next string
            (char*)combinedCommand.c_str(),  // the command
            nullptr                     // must terminat by a null pointer
    };

    

    pid_t pid = fork();
    if (pid == -1) {
        SYSCALL_FAIL("fork");
    }
    else if (pid == 0){         // It's the child process
        setpgrp();

        execv("/bin/bash", newArgv);
        // only runs if execv failed, since execv does not return on success
        SYSCALL_FAIL("execv");
    }
    else{
        FORK_NOTIFY(pid, 
        if (wait(nullptr) == -1)
            SYSCALL_FAIL("wait");
        )
    }

    this->cleanup();
    return;
}

std::string SmallShell::chprompt;
pid_t SmallShell::fg_pid = -1;

SmallShell::SmallShell() : jobs_list(JobsList::getInstance()), alias_table(AliasTable::getInstance()){
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

pid_t SmallShell::get_fg_pid(){
    return fg_pid;
}

void SmallShell::set_fg_pid(pid_t pid){
    fg_pid = pid;
}

void SmallShell::reset_fg_pid(){
    fg_pid = DEFAULT_PID;
}

bool SmallShell::is_fg_empty(){
    return fg_pid == DEFAULT_PID;
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command* SmallShell::CreateCommand(const char* cmd_line, bool* run_on_background){
    bool def;
    if (run_on_background == nullptr)
        run_on_background = &def;

    string cmd_s = _trim(std::string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    char* args[MAX_ARGS];
    int count = _parseCommandLine(cmd_line, args); // allocate memory for args

    std::map<string, Command*> builtin_cmds = {
        {"chprompt", new ChpromptCommand(cmd_line)},
        {"showpid", new ShowpidCommand(cmd_line)},
        {"pwd", new PwdCommand(cmd_line)},
        {"cd", new CdCommand(cmd_line)},
        {"jobs", new JobsCommand(cmd_line)},
        {"fg", new FgCommand(cmd_line)},
        {"quit", new QuitCommand(cmd_line)},
        {"kill", new KillCommand(cmd_line)},
        {"alias", new AliasCommand(cmd_line)},
        {"unalias", new UnAliasCommand(cmd_line)},
        {"unsetenv", new UnSetEnvCommand(cmd_line)},
        {"watchproc", new WatchProcCommand(cmd_line)}
    };

    std::map<string, Command*> special_cmds{
        {"du", new DuCommand(cmd_line)},
        {"whoami", new WhoamiCommand(cmd_line)},
        {"netinfo", new NetInfoCommand(cmd_line)}
    };

    
    Command* res = nullptr;

    
    if (checkIORedirection(cmd_line)){                      // check for io redirection or pipe usage
        res = new RedirectionCommand(cmd_line);
        *run_on_background = false;
    }
    else if (checkPipe(cmd_line)){
        res = new PipeCommand(cmd_line);
    }
    else if (builtin_cmds.find(firstWord) != builtin_cmds.end()){        // check if built-in command
        res = builtin_cmds[firstWord];
        *run_on_background = false;
    }
    else if (special_cmds.find(firstWord) != special_cmds.end()){   // check if special command
        res = special_cmds[firstWord];
    }
    else if (alias_table.query(firstWord).first){                   // check for aliases
        auto alias_expansion = alias_table.query(firstWord).second;
        string rest_of_cmd = cmd_s.substr(firstWord.length());        
        res = CreateCommand((alias_expansion + rest_of_cmd).c_str(), run_on_background);
    }
    else if (checkWildcards(cmd_line)){                             // check for external simple / complex command
        res = new complexExternalCommand(cmd_line);
    }
    else{
        res = new SimpleExternalCommand(cmd_line);
    }

    // free the space allocated for args
    for (int i = 0; i < count; ++i){
        free(args[i]);
    }
    return res;
}

void SmallShell::executeCommand(const char* cmd_line){
    bool background = _isBackgroundComamnd(cmd_line);

    char* no_bg_sign = strdup(cmd_line);
    _removeBackgroundSign(no_bg_sign);

    Command* cmd = CreateCommand(no_bg_sign, &background);

    if (background){
        JobsList::getInstance().addJob(cmd, std::string(no_bg_sign));
    }
    else{
        cmd->execute();
    }
    // TODO: needs to be changed and execute on processes that weren't fork.
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

void SimpleExternalCommand::execute(){
    prepare();

    // int stat;
    pid_t pid = fork();
    if (pid < 0)
        SYSCALL_FAIL("fork");
    else if (pid == 0){
        // child
        setpgrp();

        // search for the command in PATH (run the first occurance)
        // otherwise try to run locally
        auto path_res = is_envvar("PATH");
        if (path_res.first){
            std::stringstream path(path_res.second);
            std::string buffer;

            struct stat sb;
            while (std::getline(path, buffer, ':')){
                std::string temp_path = buffer + '/' + args[0];
                int res = stat(temp_path.c_str(), &sb);
                if (res == 0){
                    execv(temp_path.c_str(), args);
                    SYSCALL_FAIL("execv");
                    exit(-1);
                }
            }
        }

        execv(args[0], args);
        SYSCALL_FAIL("execv");
        exit(-1);
    }

    FORK_NOTIFY(pid,
        if (wait(nullptr) == -1)
            SYSCALL_FAIL("wait");
    )
    cleanup();
}
