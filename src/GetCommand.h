#ifndef GETCOMMAND_H
#define GETCOMMAND_H

#include "Command.h"

class GetCommand: public Command
{
public:
    int operator()(/*Client *client*/) const;
};

#endif

