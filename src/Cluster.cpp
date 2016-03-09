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
    parents(nullptr),
    meetWorkingBaseline(false),
    lastUnixTimeInMs(-1)
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

    if (this->toFollow(1) == CABINET_ERR) {
        logFatal("init cluster cluster_id[%d] to follow error", this->getClusterId());
        exit(1);
    }
}

Client *Cluster::createClient(const int connectFd, const string &ip, const int port) {
    ClusterClient *newClient = new ClusterClient(this->clientIdMax, connectFd, ip, port, this);

    if (this->parents->addParents(newClient) == CABINET_ERR) {
        logWarning("cluster cluster_id[%d] add cluster client to parents error", this->getClusterId());
        return nullptr;
    }

    ++this->clientIdMax;
    logNotice("cluster cluster_id[%d] create client, client_id[%d], client_connect_fd[%d] client_ip[%s]", 
            this->getClusterId(), newClient->getClientId(), connectFd, newClient->getIp().c_str());
    return (Client *)newClient;
}

int Cluster::deleteClient(Client *client) {
    if (client->getCategory() == Client::NORMAL_CLIENT) {
        if (this->parents->deleteParents((ClusterClient *)client) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] delete normal client error", this->getClusterId());
            return CABINET_ERR;
        }
        logNotice("cluster cluster_id[%d] delete normal client", this->getClusterId());
        return CABINET_OK;
    }
    if (client->getCategory() == Client::CLUSTER_CLIENT) {
        if (this->siblings->deleteSiblings((ClusterClient *)client) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] delete cluster client error", this->getClusterId());
            return CABINET_ERR;
        }
        logNotice("cluster cluster_id[%d] delete cluster client", this->getClusterId());
        return CABINET_OK;
    }
    if (client->getCategory() == Client::SERVER_CLIENT) {
        if (this->children->deleteChildren((ClusterClient *)client) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] delete server client error", this->getClusterId());
            return CABINET_ERR;
        }
        logNotice("cluster cluster_id[%d] delete server client", this->getClusterId());
        return CABINET_OK;
    }

    logFatal("cluster cluster_id[%d] can not delete unknown kind of client, broken, exit", this->getClusterId());
    return CABINET_ERR;
}

int Cluster::cron() {
    return CABINET_OK;
}

int Cluster::nextCronTime() {
    return -1;
}

/*
 *brief: send first empty appendEntry(heartbeat) to all siblings
 */
int Cluster::toLead() {
    logNotice("cluster cluster_id[%d] to lead", this->getClusterId());
    this->setClusterRole(Cluster::LEADER);

    vector<ClusterClient *> onlineSiblings = this->siblings->getOnlineSiblings();
    Command &firstAppendEntryCommand = this->commandKeeperPtr->selectCommand("appendentry");
    for (ClusterClient *sibling : onlineSiblings) {
        if ((firstAppendEntryCommand >> sibling) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] first append entry to sibling error", this->getClusterId());
            continue;
        }
    }

    return CABINET_OK;
}

int Cluster::toFollow(long newTerm) {
    logNotice("cluster cluster_id[%d] to follow", this->getClusterId());
    this->setClusterRole(Cluster::FOLLOWER);

    this->currentTerm = newTerm;
    this->lastUnixTimeInMs = Util::getCurrentTimeInMs();
    
    return CABINET_OK;
}

int Cluster::toCandidate() {
    logNotice("cluster cluster_id[%d] to candidate", this->getClusterId());
    this->setClusterRole(Cluster::CANDIDATE);

    ++this->currentTerm;
    this->votedFor = this->getClusterId();
    this->lastUnixTimeInMs = Util::getCurrentTimeInMs();

    vector<ClusterClient *> onlineSiblings = this->siblings->getOnlineSiblings();
    Command &requestVoteCommand = this->commandKeeperPtr->selectCommand("requestvote");
    for (ClusterClient *sibling : onlineSiblings) {
        if ((requestVoteCommand >> sibling) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] request vote from sibling error", this->getClusterId());
            continue;
        }
    }

    return CABINET_OK;
}

Cluster::~Cluster()
{

}
