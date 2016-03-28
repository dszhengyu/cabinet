#ifndef CLUSTERDEFAULTCOMMAND_H
#define CLUSTERDEFAULTCOMMAND_H

#ifdef CABINET
    #include "Client.h"
    class Client;
#endif

#include "Command.h"
class Command;

class ClusterDefaultCommand: public Command
{
public:
#ifdef CABINET
    int operator[](Client *client) const;
#endif
    bool isCommandValid() const {return true;}
    int commandArgc() const {return -1;}
    const char commandType() const {return 'w';}
    bool needPF() const {return true;}
};

#endif

