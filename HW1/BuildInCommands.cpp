
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
    printf("smash pid is %d\n", getpid());
    this->cleanup();
}

void pwdCommand::execute() {
    this->prepare();
    char* cwd = getcwd(nullptr, 0);  // let getcwd allocate enough memory
    if (cwd != nullptr) {
        std::cout << cwd << std::endl;
    }
    free(cwd);
    this->cleanup();
}

