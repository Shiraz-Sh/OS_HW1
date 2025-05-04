#include <iostream>
#include <signal.h>

#include "signals.hpp"
#include "Commands.hpp"

using namespace std;

extern volatile pid_t foreground_pid;

void ctrlCHandler(int sig_num) {
    std::cout << "smash: got ctrl-C" << std::endl;

    if (foreground_pid > 0) {
        if (kill(foreground_pid, SIGKILL) == 0) {
            std::cout << "smash: process " << foreground_pid << " was killed" << std::endl;
        } else {
            perror("smash error: kill failed");
        }
    }
}
