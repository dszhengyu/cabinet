#ifndef GETCOMMAND_H
#define GETCOMMAND_H

#ifdef CABINET_SERVER
    #include "Client.h"
    class Client;
#endif

#include "Command.h"
class Command;

class GetCommand: public Command
{
public:
#ifdef CABINET_SERVER
    int operator()(Client &client) const;
#endif
    bool isCommandValid() const {return true;}
    int commandArgc() const {return 1;}
    const char commandType() const {return 'r';}
};

#endif

