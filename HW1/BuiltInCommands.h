#pragma once

#include "Commands.h"

class ChpromptCommand : public BuiltInCommand {
public:
    ChpromptCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};

    virtual ~ChpromptCommand() {};

    void execute() override;
};

class ShowpidCommand : public BuiltInCommand {
public:
    ShowpidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};

    virtual ~ShowpidCommand() {};

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

class JobsCommand : public BuiltInCommand{};

class QuitCommand : public BuiltInCommand{
public:
    QuitCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};

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
