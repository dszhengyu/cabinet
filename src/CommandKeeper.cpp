#include "CommandKeeper.h"
#include "NoMatchCommand.h"

#ifndef CABINET_CLUSTER
    #include "GetCommand.h"
    #include "SetCommand.h"
    #include "DelCommand.h"
#endif

#ifdef CABINET_CLUSTER
    #include "RequestVoteCommand.h"
    #include "ReplyRequestVoteCommand.h"
    #include "AppendEntryCommand.h"
#endif

CommandKeeper::CommandKeeper() :
    commandMap()
{

}

#ifdef CABINET_CLUSTER
void CommandKeeper::createClusterCommandMap() {
    logNotice("create cluster command map");
    commandMap["nomatch"] = new NoMatchCommand();
    commandMap["requestvote"] = new RequestVoteCommand();
    commandMap["replyrequestvote"] = new ReplyRequestVoteCommand();
    commandMap["appendentry"] = new AppendEntryCommand();
}
#endif

#ifndef CABINET_CLUSTER
void CommandKeeper::createServerCommandMap() {
    logNotice("create server command map");
    commandMap["get"] = new GetCommand();
    commandMap["set"] = new SetCommand();
    commandMap["del"] = new DelCommand();
    commandMap["nomatch"] = new NoMatchCommand();
}

void CommandKeeper::createClientCommandMap() {
    logNotice("create client command map");
    commandMap["get"] = new GetCommand();
    commandMap["set"] = new SetCommand();
    commandMap["del"] = new DelCommand();
    commandMap["nomatch"] = new NoMatchCommand();
}
#endif

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
