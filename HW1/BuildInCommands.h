
#ifndef SKELETON_SMASH_BUILDINCOMMANDS_H
#define SKELETON_SMASH_BUILDINCOMMANDS_H

#include "Commands.h"

class chpromptCommand : public BuiltInCommand {
public:
    chpromptCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};

    void execute() override;
};

class showpidCommand : public BuiltInCommand {
public:
    showpidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};

    void execute() override;
};

class pwdCommand : public BuiltInCommand {
public:
    pwdCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};

    void execute() override;
};

class cdCommand : public BuiltInCommand {
public:
    cdCommand(const char *cmd_line) : BuiltInCommand(cmd_line){};

    void execute() override;

    /**
     * Saving last working directory using malloc and free of oldPWD +
     * using getcwd() system call
     * @return
     */
    bool savingLastWorkingDict();
};

#endif //SKELETON_SMASH_BUILDINCOMMANDS_H
