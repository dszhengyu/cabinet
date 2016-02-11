#include "CommandKeeper.h"
#include "GetCommand.h"
#include "SetCommand.h"
#include "DelCommand.h"
#include "NoMatchCommand.h"

CommandKeeper::CommandKeeper() :
    commandMap()
{

}

void CommandKeeper::createServerCommandMap() {
    logNotice("create server command map");
    this->createNormalCommandMap();
}

void CommandKeeper::createClientCommandMap() {
    logNotice("create client command map");
    this->createNormalCommandMap();
}

void CommandKeeper::createNormalCommandMap() {
    commandMap["get"] = new GetCommand();
    commandMap["set"] = new SetCommand();
    commandMap["del"] = new DelCommand();
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
