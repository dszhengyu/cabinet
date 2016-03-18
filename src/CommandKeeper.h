#ifndef COMMANDKEEPER_H
#define COMMANDKEEPER_H

#include <string>
#include <map>
#include "Command.h"
#include "Log.h"

using std::string;
using std::map;

class Command;

class CommandKeeper
{
public :
    typedef map<string, Command *> commandmap_t;
    CommandKeeper();
#ifdef CABINET_CLUSTER
    void createClusterCommandMap();
#endif
#ifndef CABINET_CLUSTER
    void createServerCommandMap();
    void createClientCommandMap();
#endif
    Command &selectCommand(const string &commandName);
    ~CommandKeeper();
private:
    commandmap_t commandMap;
};

#endif
