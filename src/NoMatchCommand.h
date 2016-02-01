#ifndef NOMATCHCOMMAND_H
#define NOMATCHCOMMAND_H

#include "Command.h"
#include "Client.h"

class Client;
class Command;

class NoMatchCommand: public Command
{
public:
    int operator()(Client &client) const;
};

#endif

