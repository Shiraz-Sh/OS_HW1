#include <iostream>
#include <signal.h>

#include "signals.hpp"
#include "Commands.hpp"

using namespace std;

extern volatile pid_t foreground_pid;

void ctrlCHandler(int sig_num) {
    std::cout << "smash: got ctrl-C" << std::endl;

    if (foreground_pid != -1) {
        if (kill(foreground_pid, SIGKILL) == 0) {
            std::cout << "smash: process " << foreground_pid << " was killed" << std::endl;
            foreground_pid = -1;
        } else {
            perror("smash error: kill failed");
        }
    }
}
