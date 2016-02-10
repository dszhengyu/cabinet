#include "CommandKeeper.h"
#include "GetCommand.h"
#include "NoMatchCommand.h"

CommandKeeper::CommandKeeper() :
    commandMap()
{

}

void CommandKeeper::createServerCommandMap() {
    logNotice("create server command map");
    commandMap["get"] = new GetCommand();
    commandMap["nomatch"] = new NoMatchCommand();
}

void CommandKeeper::createClientCommandMap() {
    logNotice("create client command map");
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

CommandKeeper::~CommandKeeper() {
    for (auto &mapValue : this->commandMap) {
        Command *deletingCommand = mapValue.second;
        delete deletingCommand;
    }
}
