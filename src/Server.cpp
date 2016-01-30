#include "Server.h"

Server::Server() :
    serverId(-1),
    clientIdMax(0),
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
    commandKeeperPtr = new CommandKeeper();
    commandKeeperPtr->createCommandMap();
}

Client *Server::createClient() {
    Client * client = new Client(clientIdMax, commandKeeperPtr);
    ++clientIdMax;
    return client;
}
