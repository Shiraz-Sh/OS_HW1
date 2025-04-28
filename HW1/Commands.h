// Ver: 10-4-2025
#pragma once

#include <vector>
#include <string>
#include "JobsList.h"

#define MAX_ARGS 20

#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
    // TODO: Add your data members
    const char* cmd_line;
protected:
    char* args[MAX_ARGS]; // Assumption - up to 20 args
    int count;
public:
    Command(const char *cmd_line) : cmd_line(cmd_line) {};

    virtual ~Command();

    virtual void execute() = 0;

    /**
     * prepare the cmd_line into args
     */
    virtual void prepare();
    virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char *cmd_line) : Command(cmd_line) {};

    virtual ~BuiltInCommand() {
    }
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char *cmd_line) : Command(cmd_line) {};

    virtual ~ExternalCommand() {
    }

    void execute() override {};
};

class complexExternalCommand : public ExternalCommand {
public:
    complexExternalCommand(const char* cmd_line) : ExternalCommand(cmd_line) {};

    virtual ~complexExternalCommand() {};

    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
    // TODO: Add your data members public:
    QuitCommand(const char *cmd_line, JobsList *jobs);

    virtual ~QuitCommand() {
    }

    void execute() override;
};

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    JobsCommand(const char *cmd_line, JobsList *jobs);

    virtual ~JobsCommand() {
    }

    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    KillCommand(const char *cmd_line, JobsList *jobs);

    virtual ~KillCommand() {
    }

    void execute() override;
};

class AliasCommand : public BuiltInCommand {
public:
    AliasCommand(const char *cmd_line);

    virtual ~AliasCommand() {
    }

    void execute() override;
};

class UnAliasCommand : public BuiltInCommand {
public:
    UnAliasCommand(const char *cmd_line);

    virtual ~UnAliasCommand() {
    }

    void execute() override;
};

class UnSetEnvCommand : public BuiltInCommand {
public:
    UnSetEnvCommand(const char *cmd_line);

    virtual ~UnSetEnvCommand() {
    }

    void execute() override;
};

class WatchProcCommand : public BuiltInCommand {
public:
    WatchProcCommand(const char *cmd_line);

    virtual ~WatchProcCommand() {
    }

    void execute() override;
};

class SmallShell {
private:
    // TODO: Add your data members
    static std::string chprompt;
    SmallShell();

public:
    char* oldPWD = nullptr;

    JobsList& jobsList;

    Command *CreateCommand(const char *cmd_line);

    SmallShell(SmallShell const &) = delete; // disable copy ctor

    void operator=(SmallShell const&) = delete; // disable = operator

    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        if (chprompt.empty()) {
            chprompt = "smash> ";
        }
        return instance;
    }

    ~SmallShell();

    void setChprompt(std::string name) { chprompt = name; }

    std::string getChprompt() { return chprompt; }

    void executeCommand(const char *cmd_line);

    // TODO: add extra methods as needed
};
