#include "CommandKeeper.h"
#include "GetCommand.h"
#include "NoMatchCommand.h"

CommandKeeper::CommandKeeper() :
    commandMap()
{

}

void CommandKeeper::createCommandMap() {
    Log::notice("create command map");
    commandMap["get"] = new GetCommand();
    commandMap["nomatch"] = new NoMatchCommand();
}

Command &CommandKeeper::selectCommand(const string &commandName) {
    auto commandIter = commandMap.find(commandName);
    if (commandIter == commandMap.end()) {
        return *(commandMap["nomatch"]);
    }
    return *(commandMap[commandName]);
}
