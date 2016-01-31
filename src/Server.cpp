#include "Server.h"
#include "Util.h"
#include "Const.h"
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
    Log::notice("create client, client_id[%d]", client->getClientId());
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
    
    if (Util::setNonBlock(listenfd) == CABINET_ERR) {
        Log::fatal("set listen fd non-bolck error!");
        exit(1);
    }

    Log::notice("listening on port %d", this->port);
    this->listenFd = listenfd;
}

/* 
 * brief: 获取客户端连接
 *      成功返回描述符, 失败且不是因为非阻塞原因失败, 打印日志, 返回错误
 */
int Server::getConnectFd() {
    //获取tcp/ipv4连接
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int connectFd;
    if ((connectFd = accept(this->listenFd, (struct sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
        if (errno != EWOULDBLOCK) {
            Log::warning("accept client connect error");
        }
        return CABINET_ERR;
    }

    //获取client连接信息
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ip, INET_ADDRSTRLEN);
    int port = ntohs(clientAddr.sin_port);

    Log::notice("receive client connect, client_ip[%s], client_port[%d]", ip, port);

    return connectFd;
}

Server::~Server() {
    if (this->listenFd != -1) {
        close(this->listenFd);
    }
}
