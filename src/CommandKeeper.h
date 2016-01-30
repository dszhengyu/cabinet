#ifndef COMMANDKEEPER_H
#define COMMANDKEEPER_H

#include <string>
#include <map>
#include "Command.h"
#include "GetCommand.h"
#include "Log.h"

using std::string;
using std::map;

class CommandKeeper
{
public :
    typedef map<string, Command *> commandmap_t;
    CommandKeeper();
    void createCommandMap();
private:
    commandmap_t *commandMap;
};

#endif
