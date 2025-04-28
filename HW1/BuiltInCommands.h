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

class PwdCommand : public BuiltInCommand {
public:
    PwdCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};

    virtual ~PwdCommand() {};

    void execute() override;
};

class CdCommand : public BuiltInCommand {
public:
    CdCommand(const char *cmd_line) : BuiltInCommand(cmd_line){};

    virtual ~CdCommand() {};

    void execute() override;

    /**
     * Saving last working directory using malloc and free of oldPWD +
     * using getcwd() system call
     * @return
     */
    bool savingLastWorkingDict();
};

class FgCommand : public BuiltInCommand{
public:
    FgCommand(const char *cmd_line) : BuiltInCommand(cmd_line){};

    virtual ~FgCommand() {};

    void execute() override;

    /**
     * @param jobID
     * bring the specific job to foreground and wait for him
     */
    void bringJobToForeground(int jobID);
};

class jobsCommand : public BuiltInCommand{};

class QuitCommand : public BuiltInCommand{
private:
    JobsList* jobs;
public:
    QuitCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line), jobs(jobs){};

    virtual ~QuitCommand() = default;

    void execute() override;
};

class AliasCommand : public BuiltInCommand{
public:
    AliasCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};

    virtual ~AliasCommand() = default;

    void execute() override;
};

class WatchprocCommand : public BuiltInCommand{
public:
    WatchprocCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};

    virtual ~WatchprocCommand() = default;

    void execute() override;
};
