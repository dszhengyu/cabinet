#include "Server.h"
#include "Log.h"
#include "GetCommand.h"
#include <iostream>

Server::Server() :
    serverId(-1),
    port(-1),
    listenFd(-1),
    commandMap(NULL)
{

} 

void Server::initConfig() {
    serverId = SERVER_ID;
    port = CABINET_PORT;
}

void Server::init() {
    std::cout << "init complete!" << std::endl;    
    Log::notice("init complete!\n");
    this->createCommandMap();
}

void Server::createCommandMap() {
    Log::notice("create command map\n");
    commandMap = new map<string, Command*>; 
    (*commandMap)["get"] = new GetCommand();
}
