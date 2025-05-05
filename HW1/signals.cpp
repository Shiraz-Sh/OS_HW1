#include <iostream>
#include <signal.h>

#include "signals.hpp"
#include "Commands.hpp"

void ctrlCHandler(int sig_num) {
    std::cout << "smash: got ctrl-C" << std::endl;

    if (!SmallShell::getInstance().is_fg_empty()){
        pid_t fg_pid = SmallShell::getInstance().get_fg_pid();
        if (kill(fg_pid, SIGKILL) == 0){
            std::cout << "smash: process " << fg_pid << " was killed" << std::endl;
            SmallShell::getInstance().reset_fg_pid();
        } else
            SYSCALL_FAIL("kill");
    }
}
