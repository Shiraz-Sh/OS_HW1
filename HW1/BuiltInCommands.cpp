
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

#include "BuiltInCommands.hpp"
#include "JobsList.hpp"
#include "AliasTable.hpp"
#include "envvar.hpp"
#include <iomanip>

#define KILOBYTE 1024.0
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
        SYSCALL_FAIL("getpid");
    } else {
        std::cout << "smash pid is " << pid << std::endl;
    }
    this->cleanup();
}

void PwdCommand::execute() {
    this->prepare();
    char* cwd = getcwd(nullptr, 0);  // let getcwd allocate enough memory
    if (cwd == nullptr){
        SYSCALL_FAIL("getcwd");
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
            if (chdir(temp) != 0){
                SYSCALL_FAIL("chdir");
            }
            free(temp);
        }
    } else if (args[1] != nullptr) {
        // Saving last working directory
        if (!savingLastWorkingDict()) {
            return;
        }
        // Change to given directory
        if (chdir(args[1]) != 0){
            SYSCALL_FAIL("chdir");
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
    if (smash.oldPWD == nullptr){
        SYSCALL_FAIL("getcwd");
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
    return std::stoi(s) >= 0; // convert to integer and check if > 0
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
    pid_t wpid = waitpid(smash.jobs_list.getJobById(jobID)->getJobPid(), nullptr, 0);
    if (wpid == -1){
        SYSCALL_FAIL("waitpid");
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

void KillCommand::execute() {
    this->prepare();
    // cheack arguments
    char* signumChar = args[1] + 1;
    if (count != 2) {
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
    }
    if (*args[1] != '-' || !isNumber(signumChar) || !isNumber(args[2])) {
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
    }

    int jobId = std::stoi(std::string(args[2]));
    auto job = JobsList::getInstance().getJobById(jobId);
    if (job == nullptr) {
        std::cerr << "smash error: kill: job-id " << jobId << " does not exist" << std::endl;
    }

    int signum = std::stoi(std::string(signumChar));
    pid_t jobPID = job->getJobPid();
    if (kill(jobPID, signum) == 0) {  // sucsses
        std::cout << "signal number " << signum << " was sent to pid " << jobPID << std::endl;
    } else {
        SYSCALL_FAIL("kill");
    }
    this->cleanup();
}

void AliasCommand::execute(){
    prepare();

    AliasTable& table = AliasTable::getInstance();

    // For one argument just print all aliases
    if (count == 1){
        std::cout << table;
        return;
    }

    std::vector<std::string> real_args = { std::string(args[0]) };
    if (count != 2){
        // TODO: if more than one argument provided???

        // concatenate string into a line
        std::stringstream line_s;
        line_s << args[1];
        for (int i = 2; i < count; i++){
            line_s << " " << args[i];
        }

        // reparse the command
        bool long_arg = false;
        std::stringstream arg_buffer;
        std::string line = line_s.str();
        
        for (int i = 0; i < line.size(); i++){
            char ch = line[i];

            if (!long_arg){
                if (ch == '\''){
                    long_arg = true;
                }
                else if (std::isspace(ch)){ // if starting a new word push
                    if (!arg_buffer.str().empty()){
                        real_args.push_back(arg_buffer.str());
                        arg_buffer.str(std::string());
                        arg_buffer.clear();
                    }
                    continue;
                }
                arg_buffer << ch;
            }
            else{
                arg_buffer << ch;
                if (ch == '\''){
                    long_arg = false;
                }
            }
        }

        if (!arg_buffer.str().empty()){
            real_args.push_back(arg_buffer.str());
        }
    }
    else{
        real_args.push_back(std::string(args[1]));
    }

    // Check if input is valid
    std::regex pattern(R"(^([a-zA-Z0-9_]+)='([^']*)'$)");
    std::smatch matches;
    if (!std::regex_match(real_args[1], matches, pattern)){
        std::cerr << "smash error: alias: invalid alias format" << std::endl;
        return;
    }

    std::string name = matches[1];
    std::string command = matches[2];
    table.alias(name, command.c_str());

    cleanup();
}

void UnAliasCommand::execute() {
    this->prepare();
    AliasTable& table = AliasTable::getInstance();

    if (count == 1) { // no arguments are provided
        std::cerr << "smash error: unalias: not enough arguments" << std::endl;
        this->cleanup();
        return;
    }

    for (int i = 1; args[i] != nullptr; ++i) { // loop through arguments
        if (!table.unalias(std::string(args[i]))) {
            std::cerr << "smash error: unalias: " << args[i] << " alias does not exist" << std::endl;
            break;
        }
    }

    this->cleanup();
}

bool WatchProcCommand::get_mem_usage_MB(pid_t pid, double& mem) {
    std::string path = "/proc/" + std::to_string(pid) + "/status";
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1){
        SYSCALL_FAIL("open");
        return false;
    }

    // read from file
    constexpr size_t BUF_SIZE = 8192;
    char buffer[BUF_SIZE];
    ssize_t bytes_read = read(fd, buffer, BUF_SIZE - 1);
    close(fd);
    if (bytes_read <= 0){
        SYSCALL_FAIL("read");
        return false;
    }
    buffer[bytes_read] = '\0'; // Null-terminating a String

    // Search for the line starting with "VmRSS:"
    const char* vmrss_line = std::strstr(buffer, "VmRSS:");
    if (!vmrss_line) {
        return false;
    }

    // Parse the number from the line
    int kb = 0;
    if (sscanf(vmrss_line, "VmRSS: %d", &kb) != 1) {
        return false;
    }

    mem = kb / KILOBYTE;
    return true;
}

long WatchProcCommand::get_process_cpu_time(pid_t pid) {
    std::string path = "/proc/" + std::to_string(pid) + "/stat";
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1){
        SYSCALL_FAIL("open");
        return false;
    }

    char buffer[4096];
    ssize_t sizeRead = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);
    if (sizeRead <= 0){
        SYSCALL_FAIL("read");
        return -1;
    }
    buffer[sizeRead] = '\0';

    char* ptr = buffer;
    int field = 0;
    long utime = 0, stime = 0;
    while (field < 15) {
        while (*ptr == ' ') ++ptr;
        // strol - convert a C-style string (const char*) to a long integer.
        if (field == 13) utime = strtol(ptr, &ptr, 10);
        else if (field == 14) stime = strtol(ptr, &ptr, 10);
        else strtol(ptr, &ptr, 10); // skip other fields
        ++field;
    }

    return utime + stime; // in clock ticks
}

long WatchProcCommand::get_total_cpu_time() {
    int fd = open("/proc/stat", O_RDONLY);
    if (fd == -1){
        SYSCALL_FAIL("open");
        return false;
    }

    char buffer[4096];
    ssize_t sizeRead = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);
    if (sizeRead <= 0){
        SYSCALL_FAIL("read");
        return -1;
    }
    buffer[sizeRead] = '\0';

    long total = 0;
    char label[5];   // to read the first word "cpu"
    long val;  // to store each number temporarily
    char* ptr = buffer;
    sscanf(ptr, "%s", label); // skip "cpu" and store in label - we know cpu is at first because the file starts with the CPU line
    ptr = strchr(ptr, ' ') + 1; // move pointer to first number after cpu
    // adds each number to total
    while (sscanf(ptr, "%ld", &val) == 1) {
        total += val;
        ptr = strchr(ptr, ' ');
        if (!ptr) break;  // if no more spaces, stop
        ++ptr;
    }

    return total; // in clock ticks
}

void WatchProcCommand::execute() {
    this->prepare();
    if (count > 2 || !isNumber(args[1])) {  // If more than one argument was provided / the format of the arguments isn't correct
        std::cerr << "smash error: watchproc: invalid arguments" << std::endl;
        this->cleanup();
        return;
    }

    pid_t thisPID = std::stoi(args[1]);
    if (kill(thisPID, 0) == 0) { // check if process exist and accesible
        double memUsage;
        if (!get_mem_usage_MB(thisPID, memUsage)) {
            this->cleanup();
            return;
        }
        long p1 = get_process_cpu_time(thisPID);
        long t1 = get_total_cpu_time();
        if (p1 == -1 || t1 == -1) {
            this->cleanup();
            return;
        }
        sleep(1);
        long p2 = get_process_cpu_time(thisPID);
        long t2 = get_total_cpu_time();
        if (p2 == -1 || t2 == -1) {
            this->cleanup();
            return;
        }

        double cpuUsage = 100.0 * ((double)(p2 - p1) / (double)(t2 - t1));

        std::cout << std::fixed << std::setprecision(1);
        std::cout << "PID: " << args[1]
                  << " | CPU Usage: " << cpuUsage << "%"
                  << " | Memory Usage: " << memUsage << " MB " << std::endl;
    } else if (errno == ESRCH || errno == EPERM) {
        std::cerr << "smash error: watchproc: pid " << thisPID << " does not exist" << std::endl;
    }
    else{
        SYSCALL_FAIL("kill");
    }

    this->cleanup();
}

void UnSetEnvCommand::execute(){
    prepare();

    static std::vector<std::string> deleted_variables;

    if (count == 1){
        std::cerr << "smash error: unsetenv: not enough arguments" << std::endl;
    }

    for (int i = 1; i < count; i++){
        std::string varname(args[i]);

        // check the variable exists
        if (!is_envvar(varname).first){
            std::cerr << "smash error: unsetenv: " << varname << " does not exist" << std::endl;
            return;
        }

        // remove the variable
        if (!remove_environment_variable(varname)){
            std::cerr << "Removal of environment variable " << varname << " failed" << std::endl; // This should not occur in any case! 
            return;
        }
    }

    cleanup();
}


void JobsCommand::execute(){
    JobsList::getInstance().printJobsList("[", "]");
}