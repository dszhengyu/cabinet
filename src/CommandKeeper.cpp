#include "CommandKeeper.h"

CommandKeeper::CommandKeeper() :
    commandMap(NULL)
{

}

void CommandKeeper::createCommandMap() {
    Log::notice("create command map");
    commandMap = new map<string, Command*>; 
    (*commandMap)["get"] = new GetCommand();
}

