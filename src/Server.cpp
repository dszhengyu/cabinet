#include "Server.h"
#include "Log.h"
#include <iostream>

Server::Server() :
    serverId(-1),
    port(-1),
    listenFd(-1) 
{
} 

void Server::initConfig() {
    serverId = SERVER_ID;
    port = CABINET_PORT;
}

void Server::init() {
    std::cout << "init complete!" << std::endl;    
    Log::notice("init complete!\n");
}
