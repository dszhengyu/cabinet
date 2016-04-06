#include "CommandKeeper.h"

#ifndef CABINET_CLUSTER
    #include "GetCommand.h"
    #include "SetCommand.h"
    #include "DelCommand.h"
    #include "NoMatchCommand.h"
#endif

#ifdef CABINET_CLUSTER
    #include "RequestVoteCommand.h"
    #include "ReplyRequestVoteCommand.h"
    #include "AppendEntryCommand.h"
    #include "ReplyAppendEntryCommand.h"
    #include "ClusterNodeCommand.h"
    #include "ReplyClusterNodeCommand.h"
    #include "FlushServerCommand.h"
    #include "ClusterDefaultCommand.h"
#endif

CommandKeeper::CommandKeeper() :
    commandMap()
{

}

#ifdef CABINET_CLUSTER
void CommandKeeper::createClusterCommandMap() {
    logDebug("create cluster command map");
    commandMap["nomatch"] = new ClusterDefaultCommand();
    commandMap["requestvote"] = new RequestVoteCommand();
    commandMap["replyrequestvote"] = new ReplyRequestVoteCommand();
    commandMap["appendentry"] = new AppendEntryCommand();
    commandMap["replyappendentry"] = new ReplyAppendEntryCommand();
    commandMap["clusternode"] = new ClusterNodeCommand();
    commandMap["replyclusternode"] = new ReplyClusterNodeCommand();
    commandMap["flushserver"] = new FlushServerCommand();
}
#endif

#ifndef CABINET_CLUSTER
void CommandKeeper::createServerCommandMap() {
    logDebug("create server command map");
    commandMap["get"] = new GetCommand();
    commandMap["set"] = new SetCommand();
    commandMap["del"] = new DelCommand();
    commandMap["nomatch"] = new NoMatchCommand();
}

void CommandKeeper::createClientCommandMap() {
    logDebug("create client command map");
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
