
#include "BuildInCommands.h"

using namespace std;

void chpromptCommand::execute() {
    this->prepare();
    if (count >= 2) {
        SmallShell::getInstance().setChprompt(args[1]);
    } else { // no parameters are provided
        SmallShell::getInstance().setChprompt("smash> ");
    }
    this->cleanup();
}

