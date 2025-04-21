
#include <bits/stdc++.h>
#include <unistd.h>
#include "BuildInCommands.h"
#include "JobsList.h"
#include <sys/wait.h>
#define MAX_PATH 200

using namespace std;

void chpromptCommand::execute() {
    this->prepare();
    if (count >= 2) {
        SmallShell::getInstance().setChprompt(strcat(args[1], "> "));
    } else { // no parameters are provided
        SmallShell::getInstance().setChprompt("smash> ");
    }
    this->cleanup();
}

void showpidCommand::execute() {
    this->prepare();
    printf("smash pid is %d\n", getpid()); //TODO: does getpid a systemcall? for perror
    this->cleanup();
}

void pwdCommand::execute() {
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

void cdCommand::execute() {
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

bool cdCommand::savingLastWorkingDict() {
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

bool fgCommand::isNumber(const char* s) {
    if (!s || *s == '\0') return false;
    for (int i = 0; s[i]; ++i) {
        if (!std::isdigit(s[i])) return false;
    }
    if (*s == '0' || s[0] == '-') return false;
    return true;
}

void fgCommand::execute() {
    this->prepare();
    if (count > 2 ||!isNumber(args[1])) {  // If more than one argument was provided / the format of the arguments isn't correct
        std::cerr << "smash error: fg: invalid arguments" << std::endl;
        this->cleanup();
        return;
    }

    SmallShell& smash = SmallShell::getInstance();
    if (args[1] != nullptr) {
        int givenGobID = std::stoi(args[1]);
        if (smash.jobsList.getJobById(givenGobID) == nullptr) { // If job-id was specified with a job id that does not exist
            std::cerr << "smash error: fg: job-id " << givenGobID << " does not exist" << std::endl;
            this->cleanup();
            return;
        }
        // brings the givenJobID to foreground
        bringJobToForeground(givenGobID);
    } else { // If no other argument was given
        if (smash.jobsList.isJobsListEmpty()) {
            std::cerr << "smash error: fg: jobs list is empty" << std::endl;
            this->cleanup();
            return;
        }
        // brings the job with the maximal job id in the jobs list to foregrund
        bringJobToForeground(smash.jobsList.getMaxJobID()->getJobID());
    }
    this->cleanup();
}

void fgCommand::bringJobToForeground(int jobID) {
    SmallShell& smash = SmallShell::getInstance();
    // smash waits for certain job to finish, meaning it will be brought to foreground
    pid_t wpid = waitpid(smash.jobsList.getJobById(jobID)->getJobPid(), smash.jobsList.getJobById(jobID)->getWstatus(), 0);
    if (wpid == -1) {
        perror("smash error: waitpid failed");
        return;
    }
}
