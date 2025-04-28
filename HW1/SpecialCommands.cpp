
#include "SpecialCommands.h"
#include <sys/types.h>
#include <pwd.h>
#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <sys/wait.h>
#include <cstdlib>

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
        if (strcmp(args[i], "|") == 0) {
            isSimplePipe = true;
            numOfPipe = i;
        } else if (strcmp(args[i], "|&") == 0) {
            isSimplePipe = false;
            numOfPipe = i;
        }
    }

    for (int j = 0; j < numOfPipe; j++) {
        args1[j] = args[j];
    }
    args1[numOfPipe] = nullptr;

    for (int j = (numOfPipe + 1), k = 0; j < count; j++, k++) {
        args2[k] = args[j];
    }
    args2[count - numOfPipe - 1] = nullptr;       // terminate args

    std::string cmd_line1 = joinArgs(args1);
    std::string cmd_line2 = joinArgs(args2);

    // Gets the relevant commands
    SmallShell& smash = SmallShell::getInstance();
    Command* cmd1 = smash.CreateCommand(cmd_line1.c_str());
    Command* cmd2 = smash.CreateCommand(cmd_line2.c_str());

    int pipefd[2];  // pipefd[0] for the read end and pipefd[1] for the write end
    if (pipe(pipefd) == -1) {
        perror("smash error: pipe failed");
        this->cleanup();
        return;
    }

    pid_t pid1 = fork();
    if (pid1 == -1){
        perror("smash error: fork failed");
        this->cleanup();
        return;
    } else if (pid1 == 0) {      // first son process
        // Redirect
        if (isSimplePipe) {                   // we use operator '|'
            dup2(pipefd[1], STDOUT_FILENO);     // replace stdout with pipe's write end
        } else {                               // we use operator '|&'
            dup2(pipefd[1], STDERR_FILENO);      // replace stderr with pipe’s write end
        }
        close(pipefd[0]);   // close unused read end
        close(pipefd[1]);   // close write end after dup2
        cmd1->execute();
        exit(0);
    }
    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("smash error: fork failed");
        this->cleanup();
        return;
    } else if (pid2 == 0) {       // second son process
        // Redirect
        if (isSimplePipe) {                 // we use operator '|'
            dup2(pipefd[0], STDIN_FILENO);     // replace stdin with pipe's read end
        } else {                             // we use operator '|&'
            dup2(pipefd[0], STDIN_FILENO);      // replace stdin with pipe's read end
        }
        close(pipefd[0]);
        close(pipefd[1]);
        cmd2->execute();
        exit(0);
    }

    // parent process - only the parent comes here thanks to exit()
    close(pipefd[0]);
    close(pipefd[1]);

    if (waitpid(pid1, nullptr, 0) == -1)
        perror("smash error: waitpid failed");
    if (waitpid(pid2, nullptr, 0) == -1)
        perror("smash error: waitpid failed");

    this->cleanup();
}

void RedirectionCommand::execute() {
    // TODO: implement
}
