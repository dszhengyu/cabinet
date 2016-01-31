#include "Server.h"
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

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

    //to-do print conf to log
}

void Server::init() {
    this->commandKeeperPtr = new CommandKeeper();
    this->commandKeeperPtr->createCommandMap();
    this->listenOnPort();
}

Client *Server::createClient() {
    Client * client = new Client(this->clientIdMax, commandKeeperPtr);
    ++this->clientIdMax;
    return client;
}

void Server::listenOnPort() {
    int listenfd;
    struct sockaddr_in serverAddr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        Log::fatal("create socket fail!");
        exit(1);
    }   

    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(this->port);

    if (bind(listenfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        Log::fatal("bind error!");
        exit(1);
    }   

    if (listen(listenfd, 5)< 0) {
        Log::fatal("listen port error!");
        exit(1);
    }

    Log::notice("listen on port %d", this->port);
    this->listenFd = listenfd;
}

Server::~Server() {
    if (this->listenFd != -1) {
        close(this->listenFd);
    }
}
