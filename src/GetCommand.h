#ifndef GETCOMMAND_H
#define GETCOMMAND_H

#include "Command.h"
#include "Client.h"
class Client;

class GetCommand: public Command
{
public:
    int operator()(Client &client) const;
};

#endif

