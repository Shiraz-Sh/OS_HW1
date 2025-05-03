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

#include "Commands.h"
#include "BuiltInCommands.h"
#include "JobsList.h"
#include "SpecialCommands.h"
#include "envvar.hpp"

#define MAX_ARGS 20

using namespace std;

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

string _ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s) {
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {
        args[i] = (char *) malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h
Command::~Command() {}

void Command::prepare() {
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
bool checkWildcards(const char* cmd_line) {
    while (*cmd_line != '\0') {
        if (*cmd_line == '?' || *cmd_line == '*')
            return true;
        cmd_line++;
    }
    return false;
}

bool checkIORedirection(char* args[MAX_ARGS], int count) {
    for (int i = 0; i < count; i++) {
        if (args[i] == ">" || args[i] == ">>")
            return true;
    }
    return false;
}

bool checkPipe(char* args[MAX_ARGS], int count) {
    for (int i = 0; i < count; i++) {
        if (args[i] == "|" || args[i] == "|&")
            return true;
    }
    return false;
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
        perror("smash error: fork failed");
    } else if (pid == 0) {         // It's the child process
        execv("/bin/bash", newArgv);
        // only runs if execv failed, since execv does not return on success
        perror("smash error: execv failed");
    } else {
        pid_t ret = wait(NULL);
        if (ret == -1)
            perror("smash error: wait failed");
    }

    this->cleanup();
    return;
}

std::string SmallShell::chprompt;

SmallShell::SmallShell() : jobs_list(JobsList::getInstance()), alias_table(AliasTable::getInstance()){
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command* SmallShell::CreateCommand(const char* cmd_line){
    // For example:
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    char* args[MAX_ARGS];
    int count = _parseCommandLine(cmd_line, args); // allocate memory for args

    // TODO: maybe converet this to a map?
    std::map<string, Command*> builtin_cmds{
        {"chprompt", new ChpromptCommand(cmd_line)},
        {"showpid", new ShowpidCommand(cmd_line)},
        {"pwd", new PwdCommand(cmd_line)},
        {"cd", new CdCommand(cmd_line)},
        // {"jobs", new JobsCommand(cmd_line)},
        {"fg", new FgCommand(cmd_line)},
        {"quit", new QuitCommand(cmd_line)},
        // {"kill", new KillCommand(cmd_line)},
        {"alias", new AliasCommand(cmd_line)} ,
        // {"unalias", new UnAliasCommand(cmd_line)},
        {"unsetenv", new UnSetEnvCommand(cmd_line)} //,
        // {"watchproc", new WatchProcCommand(cmd_line)}
    };

    std::map<string, Command*> special_cmds{
        // {"du", new DuCommand(cmd_line)},
        {"whoami", new WhoamiCommand(cmd_line)} //,
        // {"netinfo", new NetInfoCommand(cmd_line)},
    };

    Command* res = nullptr;
    if (builtin_cmds.find(firstWord) != builtin_cmds.end()){        // check if built-in command
        res = builtin_cmds[firstWord];
    }
    else if (special_cmds.find(firstWord) != special_cmds.end()){   // check if special command
        res = special_cmds[firstWord];
    }
    else if (checkIORedirection(args, count)){                      // check for io redirection or pipe usage
        res = new RedirectionCommand(cmd_line);
    }
    else if (checkPipe(args, count)){
        res = new PipeCommand(cmd_line);
    }
    else if (alias_table.query(firstWord).first){                   // check for aliases
        auto alias_expansion = alias_table.query(firstWord).second;
        string rest_of_cmd = cmd_s.substr(firstWord.length());
        res = CreateCommand((alias_expansion + rest_of_cmd).c_str());
    }
    else if (checkWildcards(cmd_line)){                             // check for external simple / complex command
        res = new complexExternalCommand(cmd_line);
    }
    else{
        // TODO: return simple external command
        res = new SimpleExternalCommand(cmd_line);
        // std::cout << cmd_line << std::endl;
    }

    // free the space allocated for args
    for (int i = 0; i < count; ++i){
        free(args[i]);
    }
    return res;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    Command* cmd = CreateCommand(cmd_line);
    cmd->execute(); // TODO: needs to be changed and execute on processes that weren't fork.
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

void SimpleExternalCommand::execute(){
    prepare();

    for (int i = 0; i < count; i++){
        std::cout << args[i] << std::endl;
    }

    // int stat;
    pid_t pid = fork();
    if (pid < 0)
        perror("could not fork");
    else if (pid == 0){
        // child

        // this should catch all the explicit executions of files
        if (args[0][0] == '.' || args[0][0] == '/'){
            execv(args[0], args);
            perror("execv failed");
            exit(-1);
        }

        // search for the command in PATH (run the first occurance)
        // otherwise try to run locally
        auto path_res = is_envvar("PATH");
        if (path_res.first){
            std::stringstream path(path_res.second);
            std::string buffer;

            struct stat sb;
            while (std::getline(path, buffer, ':')){
                std::string temp_path = buffer + '/' + args[0];
                if (stat(temp_path.c_str(), &sb) == 0){
                    execv(temp_path.c_str(), args);
                    perror("execv failed");
                    exit(-1);
                }
            }
        }
        else{
            // We wouldn't want to get here...
            execv(args[0], args);
            perror("execv failed");
            exit(-1);
        }
    }
    else if (wait(nullptr) < 0)
        perror("wait failed");

    cleanup();
}