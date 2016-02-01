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
    void createCommandMap();
    Command &selectCommand(const string &commandName);
private:
    commandmap_t commandMap;
};

#endif
