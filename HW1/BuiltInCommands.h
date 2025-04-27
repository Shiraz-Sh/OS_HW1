#pragma once

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

class fgCommand : public BuiltInCommand{
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

class jobsCommand : public BuiltInCommand{};

class quitCommand : public BuiltInCommand{
private:
    JobsList* jobs;
public:
    quitCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line), jobs(jobs){};

    virtual ~quitCommand() = default;

    void execute() override;
};

class aliasCommand : public BuiltInCommand{
public:
    aliasCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};

    virtual ~aliasCommand() = default;

    void execute() override;
};

