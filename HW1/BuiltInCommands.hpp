#pragma once

#include "Commands.hpp"

class ChpromptCommand : public BuiltInCommand {
public:
    ChpromptCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~ChpromptCommand() = default;

    void execute() override;
};

class ShowpidCommand : public BuiltInCommand {
public:
    ShowpidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~ShowpidCommand() = default;

    void execute() override;
};

class PwdCommand : public BuiltInCommand {
public:
    PwdCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~PwdCommand() = default;

    void execute() override;
};

class CdCommand : public BuiltInCommand {
public:
    CdCommand(const char *cmd_line) : BuiltInCommand(cmd_line){}

    virtual ~CdCommand() = default;

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
    FgCommand(const char *cmd_line) : BuiltInCommand(cmd_line){}

    virtual ~FgCommand() = default;

    void execute() override;

    /**
     * @param jobID
     * bring the specific job to foreground and wait for him
     */
    void bringJobToForeground(int jobID);
};

class JobsCommand : public BuiltInCommand{
public:
    JobsCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}

    virtual ~JobsCommand() = default;

    void execute() override;
};

class QuitCommand : public BuiltInCommand{
public:
    QuitCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}

    virtual ~QuitCommand() = default;

    void execute() override;
};

class AliasCommand : public BuiltInCommand{
public:
    AliasCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}

    virtual ~AliasCommand() = default;

    void execute() override;
};

class UnAliasCommand : public BuiltInCommand {
public:
    UnAliasCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}

    virtual ~UnAliasCommand() = default;

    void execute() override;
};

class WatchProcCommand : public BuiltInCommand{
public:
    WatchProcCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}

    virtual ~WatchProcCommand() = default;

    long get_total_cpu_time();

    long get_process_cpu_time(pid_t pid);

    bool get_mem_usage_MB(pid_t pid, double& mem);

    void execute() override;
};

class UnSetEnvCommand : public BuiltInCommand{
public:
    UnSetEnvCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}

    virtual ~UnSetEnvCommand() = default;

    void execute() override;
};