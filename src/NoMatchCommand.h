#ifndef NOMATCHCOMMAND_H
#define NOMATCHCOMMAND_H

#ifdef CABINET
    #include "Client.h"
    class Client;
#endif

#include "Command.h"
class Command;

class NoMatchCommand: public Command
{
public:
#ifdef CABINET_CLUSTER
    int operator>>(Client *client) {return 0;}
    int operator[](Client *client) {return 0;}
#endif
#ifdef CABINET
    int operator()(Client *client) const;
#endif
    bool isCommandValid() const {return false;}
    int commandArgc() const {return 0;}
    const char commandType() const {return 'r';}
    bool needPF() const {return false;}
};

#endif

