#ifndef NOMATCHCOMMAND_H
#define NOMATCHCOMMAND_H

#ifdef CABINET_SERVER
    #include "Client.h"
    class Client;
#endif

#include "Command.h"
class Command;

class NoMatchCommand: public Command
{
public:
#ifdef CABINET_SERVER
    int operator()(Client *client) const;
#endif
    bool isCommandValid() const {return false;}
    int commandArgc() const {return 0;}
    const char commandType() const {return 'r';}
};

#endif

