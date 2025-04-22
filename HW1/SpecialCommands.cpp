
#include "SpecialCommands.h"
#include <sys/types.h>
#include <pwd.h>
#include <iostream>
#include <unistd.h>
#include <cstdio>

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
