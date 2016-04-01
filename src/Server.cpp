#include "Server.h"
#include "Util.h"
#include "ServerClient.h"
#include "Configuration.h"
#include "Entry.h"
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <exception>

Server::Server() :
    Cabinet(),
    serverId(-1),
    db(nullptr),
    pfName()
{

} 

void Server::initConfig() {
    Configuration conf;
    if (conf.loadConfiguration() == CABINET_ERR) {
        logFatal("load conf error, exit");
        exit(1);
    }
    try{
        this->serverId = std::stoi(conf["SERVER_ID"]);
        this->port = std::stoi(conf["SERVER_PORT"]);;
        this->pfName = conf["SERVER_PF_NAME"];
        this->allowPF = conf["SERVER_PF"];
    } catch (std::exception &e) {
        logFatal("read conf fail, receive exception, what[%s]", e.what());
        exit(1);
    }
}

void Server::init() {
    if (Util::daemonize() == CABINET_ERR) {
        logFatal("daemonize fail, exit");
        exit(1);
    }

    this->commandKeeperPtr = new CommandKeeper();
    this->commandKeeperPtr->createServerCommandMap();

    while ((this->listenFd = this->listenOnPort(this->port)) == CABINET_ERR) {
        logFatal("server listen on port[%d] error, keep trying", this->port);
        sleep(5);
    }

    this->eventPoll = new EventPoll(this);
    if (this->eventPoll->initEventPoll() == CABINET_ERR) {
        logFatal("create event poll error");
        exit(1);
    }

    this->eventPoll->pollListenFd(this->getListenFd());

    this->db = new DataBase();

    if (this->allowPF == string("true")) {
        logNotice("persistence is require");
        this->pf = new PersistenceFile(this->pfName, this->pfName + ".temp");
        this->importPF();
    }
    logNotice("init server done");
    #include "CabinetLogo.h"
    logNotice(cabinet_server_logo, this->port, this->allowPF.c_str());
}

Client *Server::createClient(int listenFd) {
    if (listenFd != this->getListenFd()) {
        logWarning("get fault listen fd to create client");
        return nullptr;
    }
    int connectFd = 0;
    string ip;
    int port = 0;
    if ((connectFd = this->getConnectFd(listenFd, ip, port)) == CABINET_ERR) {
        logWarning("get connect fd error");
        return nullptr;
    }
    Client * client = new ServerClient(this->clientIdMax, connectFd, ip, port, this->db, this);
    ++this->clientIdMax;
    logNotice("create client, client_id[%d], client_connect_fd[%d] client_ip[%s]", 
            client->getClientId(), connectFd, client->getIp().c_str());
    return client;
}


/* 
 * brief: 导入持久化文件
 */
int Server::importPF() {
    logNotice("start import persistence file");
    //create a client, set client category
    Client *pFClient = new ServerClient(this->clientIdMax++, -1, string(), 0, this->db, this);
    pFClient->setCategory(Client::LOCAL_PF_CLIENT);

    //start loop, read one entry from pf
    Entry entry;
    while (this->pf->getNextPFEntry(entry) != CABINET_ERR) {
        //feed the client, resolve it, execute it
        const string &curEntryContent = entry.getContent();
        //logDebug("--importing persistence file, current entry[\n%s]", curEntryContent.c_str());
        pFClient->fillReceiveBuf(curEntryContent); 
        if (pFClient->resolveReceiveBuf() == CABINET_ERR) {
            logWarning("import persistence file error, revolve error, current entry[\n%s]", curEntryContent.c_str());
            return CABINET_ERR;
        }

        if (pFClient->isReceiveComplete() == true) {
            if (pFClient->executeCommand() == CABINET_ERR) {
                logWarning("import persistence file error, execute error, current entry[\n%s]", curEntryContent.c_str());
                return CABINET_ERR;
            }
        }
        pFClient->resetClient();
    }
    logNotice("end import persistence file");
    return CABINET_OK;
}

Server::~Server() {
}
