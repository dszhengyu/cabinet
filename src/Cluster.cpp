#include "Cluster.h"
#include "Configuration.h"
#include "Util.h"
#include "Log.h"
#include "Const.h"
#include <unistd.h>
#include <exception>

Cluster::Cluster():
    clusterId(-1),
    role(Cluster::FOLLOWER),
    currentTerm(0),
    votedFor(-1),
    commitIndex(0),
    lastApplied(0),
    siblings(nullptr),
    children(nullptr),
    parents(nullptr)
{
}

void Cluster::initConfig() {
    Configuration conf;
    if (conf.loadConfiguration() == CABINET_ERR) {
        logFatal("load conf error, exit");
        exit(1);
    }
    try{
        this->clusterId = std::stoi(conf["CLUSTER_ID"]);
        this->port = std::stoi(conf["CLUSTER_PORT"]);;
    } catch (std::exception &e) {
        logFatal("read conf fail, receive exception, what[%s]", e.what());
        exit(1);
    }

    this->siblings = new Siblings();
    if (this->siblings->recognizeSiblings(conf) == CABINET_ERR) {
        logFatal("recognize siblings error, exit");
        exit(1);
    }

    this->children = new Children();
    if (this->children->recognizeChildren(conf) == CABINET_ERR) {
        logFatal("recognize children error, exit");
        exit(1);
    }

    this->parents = new Parents();
}

void Cluster::init() {
    if (Util::daemonize() == CABINET_ERR) {
        logFatal("daemonize fail, exit");
        exit(1);
    }

    this->commandKeeperPtr = new CommandKeeper();
    this->commandKeeperPtr->createClusterCommandMap();

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
}

Client *Cluster::createClient(const int connectFd, const string &ip, const int port) {

    return nullptr;
}

int Cluster::deleteClient(Client *client) {

    return CABINET_OK;
}

int Cluster::cron() {
    return CABINET_OK;
}

int Cluster::nextCronTime() {
    return -1;
}

Cluster::~Cluster()
{

}
