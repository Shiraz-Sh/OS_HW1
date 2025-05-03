// Ver: 10-4-2025
#pragma once

#include <vector>
#include <string>
#include <string.h>
#include <utility>

#include "JobsList.hpp"
#include "AliasTable.hpp"

#define MAX_ARGS 20

#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define SYSCALL_FAIL(name) \
    perror(std::string("smash error: " + std::string(name) + " failed").c_str())

class AliasTable;

class Command{
    // TODO: Add your data members
    char* cmd_line;
protected:
    char* args[MAX_ARGS]; // Assumption - up to 20 args
    int count;
public:
    Command(const char *cmd_line) : cmd_line(strdup(cmd_line)) {}

    virtual ~Command(){
        free(cmd_line);
    }

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
    BuiltInCommand(const char *cmd_line) : Command(cmd_line) {}

    virtual ~BuiltInCommand() = default;
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char *cmd_line) : Command(cmd_line) {}

    virtual ~ExternalCommand() = default;

    void execute() override {};
};

class complexExternalCommand : public ExternalCommand {
public:
    complexExternalCommand(const char* cmd_line) : ExternalCommand(cmd_line) {}

    virtual ~complexExternalCommand() = default;

    void execute() override;
};

class SimpleExternalCommand : public ExternalCommand{
public:
    SimpleExternalCommand(const char* cmd_line) : ExternalCommand(cmd_line){}

    virtual ~SimpleExternalCommand() = default;

    void execute() override;
};


class JobsList;

class SmallShell {
private:
    // TODO: Add your data members
    static std::string chprompt;
    SmallShell();

public:
    char* oldPWD = nullptr;

    JobsList& jobs_list;
    AliasTable& alias_table;
    
    Command* CreateCommand(const char* cmd_line, bool* run_on_background = nullptr);

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
