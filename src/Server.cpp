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
    db(nullptr),
    pf(nullptr)
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

    if (PF) {
        logNotice("persistence is require");
        this->pf = new PersistenceFile();
        this->importPF();
    }
    logNotice("init server done");
}

Client *Server::createClient(int connectFd) {
    Client * client = new Client(this->clientIdMax, this->commandKeeperPtr, connectFd, this->eventPoll, this->db, this->pf);
    ++this->clientIdMax;
    logNotice("create client, client_id[%d], client_connect_fd[%d]", client->getClientId(), connectFd);
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

/* 
 * brief: 导入持久化文件
 */
int Server::importPF() {
    logNotice("start import persistence file");
    //create a client, set client category
    Client *pFClient = this->createClient(-1);
    pFClient->setCategory(Client::LOCAL_PF_CLIENT);

    //init read pf
    if (this->pf->initReadPF() == CABINET_ERR) {
        logWarning("init persistence file error");
        return CABINET_ERR;
    }

    //start loop, read one entry from pf
    while (this->pf->readNextPFEntry() != CABINET_ERR) {
        //feed the client, resolve it, execute it
        const string &curPFEntry = this->pf->getCurPFEntry();
        logDebug("--importing persistence file, current entry[\n%s]", curPFEntry.c_str());
        pFClient->fillReceiveBuf(curPFEntry); 
        if (pFClient->resolveReceiveBuf() == CABINET_ERR) {
            logWarning("import persistence file error, revolve error, current entry[\n%s]", curPFEntry.c_str());
            return CABINET_ERR;
        }

        if (pFClient->isReceiveComplete() == true) {
            if (pFClient->executeCommand() == CABINET_ERR) {
                logWarning("import persistence file error, execute error, current entry[\n%s]", curPFEntry.c_str());
                return CABINET_ERR;
            }
        }
    }
    this->pf->endReadPF();
    logNotice("end import persistence file");
    return CABINET_OK;
}

Server::~Server() {
    if (this->listenFd != -1) {
        close(this->listenFd);
    }
    delete this->eventPoll;
    delete this->commandKeeperPtr;
}
