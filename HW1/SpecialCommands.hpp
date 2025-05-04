#pragma once

#include "Commands.hpp"

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line) : Command(cmd_line) {};

    virtual ~RedirectionCommand() {
    }

    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line) : Command(cmd_line) {};

    virtual ~PipeCommand() {
    }

    void execute() override;
};

class DiskUsageCommand : public Command {
public:
    DiskUsageCommand(const char *cmd_line) : Command(cmd_line) {};

    virtual ~DiskUsageCommand() {
    }

    void execute() override;
};

class WhoamiCommand : public Command {
public:
    WhoamiCommand(const char *cmd_line) : Command(cmd_line) {};

    virtual ~WhoamiCommand() {
    }

    void execute() override;
};

class NetInfoCommand : public Command {
    // TODO: Add your data members **BONUS: 10 Points**
public:
    NetInfoCommand(const char* cmd_line) : Command(cmd_line){}

    virtual ~NetInfoCommand() = default;

    void execute() override;
};

class DuCommand : public Command {
public:
    DuCommand(const char* cmd_line) : Command(cmd_line){}

    virtual ~DuCommand() = default;

    void execute() override;
};
