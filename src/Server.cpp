#include "Server.h"
#include "Util.h"
#include "Const.h"
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

Server::Server() :
    serverId(-1),
    clientIdMax(0),
    port(-1),
    listenFd(-1),
    commandKeeperPtr(nullptr),
    eventPoll(nullptr),
    db(nullptr)
{

} 

void Server::initConfig() {
    serverId = SERVER_ID;
    port = CABINET_PORT;

    //to-do print conf to log
}

void Server::init() {
    this->commandKeeperPtr = new CommandKeeper();
    this->commandKeeperPtr->createServerCommandMap();

    if (this->listenOnPort() == CABINET_ERR) {
        logFatal("listen on port error");
        exit(1);
    }

    this->eventPoll = new EventPoll(this);
    if (this->eventPoll->initEventPoll() == CABINET_ERR) {
        logFatal("create event poll error");
        exit(1);
    }

    this->eventPoll->pollListenFd(this->getListenFd());

    this->db = new DataBase();
}

Client *Server::createClient(int connectFd) {
    Client * client = new Client(this->clientIdMax, this->commandKeeperPtr, connectFd, this->eventPoll, this->db);
    ++this->clientIdMax;
    logNotice("create client, client_id[%d]", client->getClientId());
    return client;
}

int Server::listenOnPort() {
    int listenfd;
    struct sockaddr_in serverAddr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        logFatal("create socket fail!");
        return CABINET_ERR;
    }   

    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(this->port);

    if (bind(listenfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        logFatal("bind error!");
        return CABINET_ERR;
    }   

    if (listen(listenfd, 5)< 0) {
        logFatal("listen port error!");
        return CABINET_ERR;
    }
    
    if (Util::setNonBlock(listenfd) == CABINET_ERR) {
        logFatal("set listen fd non-bolck error!");
        return CABINET_ERR;
    }

    logNotice("listening on port %d", this->port);
    this->listenFd = listenfd;
    return CABINET_OK;
}

/* 
 * brief: 获取客户端连接
 *      成功返回描述符, 失败且不是因为非阻塞原因失败, 打印日志, 返回错误
 */
int Server::getConnectFd() {
    logDebug("server get client connect");
    //获取tcp/ipv4连接
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int connectFd;
    if ((connectFd = accept(this->listenFd, (struct sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
        if (errno != EWOULDBLOCK) {
            logWarning("accept client connect error");
        }
        return CABINET_ERR;
    }

    if (Util::setNonBlock(connectFd) == CABINET_ERR) {
        logWarning("set connect fd non-bolck error!");
        close(connectFd);
        return CABINET_ERR;
    }

    //获取client连接信息
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ip, INET_ADDRSTRLEN);
    int port = ntohs(clientAddr.sin_port);

    logNotice("receive client connect, client_ip[%s], client_port[%d]", ip, port);

    return connectFd;
}

void Server::onFire() const {
    logDebug("server on fire");
    this->eventPoll->processEvent();
}

Server::~Server() {
    if (this->listenFd != -1) {
        close(this->listenFd);
    }
    delete this->eventPoll;
    delete this->commandKeeperPtr;
}
