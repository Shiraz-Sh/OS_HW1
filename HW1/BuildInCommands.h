
#ifndef SKELETON_SMASH_BUILDINCOMMANDS_H
#define SKELETON_SMASH_BUILDINCOMMANDS_H

#include "Commands.h"

class chpromptCommand : public BuiltInCommand {
public:
    chpromptCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};

    virtual ~chpromptCommand() {};

    void execute() override;
};

class showpidCommand : public BuiltInCommand {
public:
    showpidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};

    virtual ~showpidCommand() {};

    void execute() override;
};

class pwdCommand : public BuiltInCommand {
public:
    pwdCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};

    virtual ~pwdCommand() {};

    void execute() override;
};

class cdCommand : public BuiltInCommand {
public:
    cdCommand(const char *cmd_line) : BuiltInCommand(cmd_line){};

    virtual ~cdCommand() {};

    void execute() override;

    /**
     * Saving last working directory using malloc and free of oldPWD +
     * using getcwd() system call
     * @return
     */
    bool savingLastWorkingDict();
};

class fgCommand : public BuiltInCommand {
public:
    fgCommand(const char *cmd_line) : BuiltInCommand(cmd_line){};

    virtual ~fgCommand() {};

    void execute() override;

    /**
     * @param jobID
     * bring the specific job to foreground and wait for him
     */
    void bringJobToForeground(int jobID);

    /**
     * cheak if a specific string is a positive number
     * @param s
     * @return
     */
    bool isNumber(const char* s);
};

#endif //SKELETON_SMASH_BUILDINCOMMANDS_H
