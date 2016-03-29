#ifndef FLUSHSERVERCOMMAND_H
#define FLUSHSERVERCOMMAND_H

#ifdef CABINET
    #include "Client.h"
    class Client;
#endif

#include "Command.h"
class Command;

class FlushServerCommand: public Command
{
public:
#ifdef CABINET
    int operator>>(Client *client) const;
    int operator[](Client *client) const;
#endif
    bool isCommandValid() const {return true;}
    int commandArgc() const {return 1;}
    const char commandType() const {return 'r';}
    bool needPF() const {return false;}
};

#endif

