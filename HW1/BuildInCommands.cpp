
#include <bits/stdc++.h>
#include <unistd.h>
#include "BuildInCommands.h"
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
