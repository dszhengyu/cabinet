#ifndef GETCOMMAND_H
#define GETCOMMAND_H

#ifdef CABINET
    #include "Client.h"
    class Client;
#endif

#include "Command.h"
class Command;

class GetCommand: public Command
{
public:
#ifdef CABINET
    int operator()(Client *client) const;
#endif
    bool isCommandValid() const {return true;}
    int commandArgc() const {return 1;}
    const char commandType() const {return 'r';}
    bool needPF() const {return false;}
};

#endif

