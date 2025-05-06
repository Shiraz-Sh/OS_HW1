#pragma once

#include "Commands.hpp"

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const std::string& cmd_line) : Command(cmd_line){};

    virtual ~RedirectionCommand() = default;

    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const std::string& cmd_line) : Command(cmd_line){};

    virtual ~PipeCommand() = default;

    void execute() override;
};

class DiskUsageCommand : public Command {
public:
    DiskUsageCommand(const std::string& cmd_line) : Command(cmd_line){};

    virtual ~DiskUsageCommand() = default;

    void execute() override;
};

class WhoamiCommand : public Command {
public:
    WhoamiCommand(const std::string& cmd_line) : Command(cmd_line){};

    virtual ~WhoamiCommand() = default;

    void execute() override;
};

class NetInfoCommand : public Command {
    // TODO: Add your data members **BONUS: 10 Points**
public:
    NetInfoCommand(const std::string& cmd_line) : Command(cmd_line){}

    virtual ~NetInfoCommand() = default;

    void execute() override;
};

class DuCommand : public Command {
public:
    DuCommand(const std::string& cmd_line) : Command(cmd_line){}

    virtual ~DuCommand() = default;

    void execute() override;
};
