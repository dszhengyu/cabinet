#ifndef CLUSTERNODECOMMAND_H
#define CLUSTERNODECOMMAND_H

#ifdef CABINET
    #include "Client.h"
    class Client;
#endif

#include "Command.h"
class Command;

class ClusterNodeCommand: public Command
{
public:
#ifdef CABINET
    int operator>>(Client *client) const;
    int operator[](Client *client) const;
#endif
    bool isCommandValid() const {return true;}
    int commandArgc() const {return 2;}
    const char commandType() const {return 'r';}
    bool needPF() const {return false;}
};

#endif

