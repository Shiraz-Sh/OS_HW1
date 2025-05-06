// Ver: 10-4-2025
#pragma once

#include <vector>
#include <string>
#include <string.h>
#include <utility>

#include "JobsList.hpp"
#include "AliasTable.hpp"
#include "global_utils.hpp"

#define MAX_ARGS 20

#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class AliasTable;

class Command{
    // TODO: Add your data members
protected:
    std::string cmd_line;
    char* args[MAX_ARGS]; // Assumption - up to 20 args
    int count;
public:
    Command(const std::string& cmd_line) : cmd_line(cmd_line){}

    virtual ~Command() = default;

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
    BuiltInCommand(const std::string& cmd_line) : Command(cmd_line){}

    virtual ~BuiltInCommand() = default;

    void execute() override = 0;
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const std::string& cmd_line) : Command(cmd_line){}

    virtual ~ExternalCommand() = default;

    void execute() override = 0;
};

class complexExternalCommand : public ExternalCommand {
public:
    complexExternalCommand(const std::string& cmd_line) : ExternalCommand(cmd_line){}

    virtual ~complexExternalCommand() = default;

    void execute() override;
};

class SimpleExternalCommand : public ExternalCommand{
public:
    SimpleExternalCommand(const std::string& cmd_line) : ExternalCommand(cmd_line){}

    virtual ~SimpleExternalCommand() = default;

    void execute() override;
};


class JobsList;

class SmallShell {
private:
    // TODO: Add your data members
    static std::string chprompt;
    static pid_t fg_pid;
    static const pid_t DEFAULT_PID = -1;
    SmallShell();

public:
    char* oldPWD = nullptr;

    JobsList& jobs_list;
    AliasTable& alias_table;
    
    Command* CreateCommand(const std::string& cmd_line, bool* run_on_background = nullptr);

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

    void executeCommand(const char* cmd_line);

    void set_fg_pid(pid_t pid);
    void reset_fg_pid();
    pid_t get_fg_pid();
    bool is_fg_empty();

    // TODO: add extra methods as needed
};
