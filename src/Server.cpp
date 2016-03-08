#include "Server.h"
#include "Util.h"
#include "ServerClient.h"
#include "Configuration.h"
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <exception>

Server::Server() :
    Cabinet(),
    serverId(-1),
    db(nullptr)
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
    #include "CabinetLogo.h"
    logNotice(cabinet_server_logo, this->port, ((PF == true) ? "enabled" : "disabled"));
}

Client *Server::createClient(const int connectFd, const string &ip, const int port) {
    Client * client = new ServerClient(this->clientIdMax, this->commandKeeperPtr, connectFd, ip, port,
            this->eventPoll, this->pf, this->db);
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
    Client *pFClient = this->createClient(-1, string(), -1);
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
        pFClient->resetClient();
    }
    this->pf->endReadPF();
    logNotice("end import persistence file");
    return CABINET_OK;
}

Server::~Server() {
}
