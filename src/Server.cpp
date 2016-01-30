#include "Server.h"

Server::Server() :
    serverId(-1),
    port(-1),
    listenFd(-1),
    commandKeeperPtr(NULL)
{

} 

void Server::initConfig() {
    serverId = SERVER_ID;
    port = CABINET_PORT;
}

void Server::init() {
    //commandKeeperPtr = make_shared<CommandKeeper>(new CommandKeeper());
    commandKeeperPtr = new CommandKeeper();
    commandKeeperPtr->createCommandMap();
}
