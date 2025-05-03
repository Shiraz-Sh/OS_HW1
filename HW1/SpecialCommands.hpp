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

class NetInfo : public Command {
    // TODO: Add your data members **BONUS: 10 Points**
public:
    NetInfo(const char *cmd_line) : Command(cmd_line) {};

    virtual ~NetInfo() {
    }

    void execute() override;
};
