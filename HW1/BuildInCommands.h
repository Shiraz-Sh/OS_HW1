
#ifndef SKELETON_SMASH_BUILDINCOMMANDS_H
#define SKELETON_SMASH_BUILDINCOMMANDS_H

#include "Commands.h"

class chpromptCommand : public BuiltInCommand {
public:
    chpromptCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {};

    void execute() override;
};

#endif //SKELETON_SMASH_BUILDINCOMMANDS_H
