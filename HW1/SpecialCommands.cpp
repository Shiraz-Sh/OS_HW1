
#include "SpecialCommands.h"
#include <sys/types.h>
#include <pwd.h>
#include <iostream>
#include <unistd.h>
#include <cstdio>

#define MAX_ARGS 20

void whoamiCommand::execute() {
    this->prepare();
    // getuid() gets the current userID and getpwuid uses the ID to get details on user
    struct passwd *user = getpwuid(getuid());
    if (user == nullptr) {
        perror("smash error: getpwuid failed");
    } else {
        std::cout << user->pw_name << " " << user->pw_dir << std::endl;
    }
    this->cleanup();
}

std::string joinArgs(char** args) {
    std::string result;
    for (int i = 0; args[i] != nullptr; ++i) {
        if (i > 0) {
            result += " ";
        }
        result += args[i];
    }
    return result;
}

void PipeCommand::execute() {
    this->prepare();

    // Seperate the two commands -
    char* args1[count];
    char* args2[count];
    int numOfPipe;
    bool isSimplePipe;
    for (int i = 0; i < count; i++) {
        if (args[i] == "|" || args[i] == "|&") {
            numOfPipe = i;
            if (args[i] == "|")
                isSimplePipe = true;
            isSimplePipe = false;
        }
    }
    for (int j = 0; j < numOfPipe; j++) {
        args1[j] = args[j];
    }
    for (int j = (numOfPipe + 1); j < count; j++) {
        args2[j] = args[j];
    }
    std::string cmd_line1 = joinArgs(args1);
    std::string cmd_line2 = joinArgs(args2);

    // Gets the relevant commands
    SmallShell& smash = SmallShell::getInstance();
    Command* cmd1 = smash.CreateCommand(cmd_line1.c_str());
    Command* cmd2 = smash.CreateCommand(cmd_line2.c_str());

//    if (isSimplePipe) {         // we use operator '|'
//
//    } else {                    // we use operator '|&'
//
//    }

    this->cleanup();
}

void RedirectionCommand::execute() {
    //TODO: implement
}
