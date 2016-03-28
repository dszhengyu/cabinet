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
    lastEntryIndex(0),
    lastApplied(0),
    siblings(nullptr),
    children(nullptr),
    parents(nullptr),
    meetWorkingBaseline(false),
    lastUnixTimeInMs(-1),
    hz(10),
    electionTimeout(100),
    receiveVotes(0),
    winVoteBaseline(0),
    clusterPort(-1),
    clusterListenFd(-1)
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
        this->hz = std::stoi(conf["CLUSTER_HZ"]);
        this->electionTimeout = std::stoi(conf["CLUSTER_ELECTION_TIMEOUT"]);
        this->winVoteBaseline = std::stoi(conf["CLUSTER_TOTAL_NUMBER"]) / 2 + 1;
        this->clusterPort = std::stoi(conf["CLUSTER_CLUSTER_PORT"]);
    } catch (std::exception &e) {
        logFatal("read conf fail, receive exception, what[%s]", e.what());
        exit(1);
    }

    this->siblings = new Siblings(this);
    if (this->siblings->recognizeSiblings(conf) == CABINET_ERR) {
        logFatal("recognize siblings error, exit");
        exit(1);
    }

    this->children = new Children(this);
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

    if ((this->listenFd = this->listenOnPort(this->port)) == CABINET_ERR) {
        logFatal("listen on port error, port[%d]", this->port);
        exit(1);
    }

    if ((this->clusterListenFd = this->listenOnPort(this->clusterPort)) == CABINET_ERR) {
        logFatal("listen on cluster port error, cluster port[%d]", this->clusterPort);
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

Client *Cluster::createClient(int listenFd) {
    int connectFd = 0;
    string ip;
    int port = 0;
    if ((connectFd = this->getConnectFd(listenFd, ip, port)) == CABINET_ERR) {
        logWarning("get connect fd error");
        return nullptr;
    }

    //through listen fd depend which group to in, parents or siblings
    ClusterClient *newClient = nullptr;
    if (listenFd == this->getListenFd()) {
        logNotice("cluster cluster_id[%d] add new client", this->getClusterId());
        newClient = this->createNormalClient(connectFd, ip, port);
        if (this->parents->addParents(newClient) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] add cluster client to parents error", this->getClusterId());
            return nullptr;
        }
    }
    else if (listenFd == this->clusterListenFd) {
        logNotice("cluster cluster_id[%d] add new cluster node", this->getClusterId());
        newClient = this->createNormalClient(connectFd, ip, port);
        newClient->setCategory(Client::CLUSTER_CLIENT);
        if (this->siblings->addSiblings(newClient) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] add cluster client to siblings error", this->getClusterId());
            return nullptr;
        }
    }
    else {
        logFatal("should not get this listen fd, please check");
        exit(1);
    }

    return (Client *)newClient;
}

ClusterClient *Cluster::createNormalClient(int connectFd, const string &ip, const int port) {
    ClusterClient *newClient = new ClusterClient(this->clientIdMax++, connectFd, ip, port, this);
    return newClient;
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

/*
 *brief: 1. 首先检查siblings和children是否连接正常
 *          否, 则更改为follower状态, 连接服务器
 *       2. 根据不同角色做不同的事情
 */
int Cluster::cron() {
    //judge whether meet working baseline
    this->meetWorkingBaseline = this->siblings->satisfyWorkingBaseling() && this->children->satisfyWorkingBaseling();
    //reconnect if baseline not achieved
    if (!this->meetWorkingBaseline) {
        logWarning("cluster cluster_id[%d] could not meet working baseline", this->getClusterId());
        if (this->toFollow(this->currentTerm) == CABINET_ERR) {
            logFatal("cluster cluster_id[%d] change to follower error", this->getClusterId());
            exit(1);
        }
        if (!this->siblings->satisfyWorkingBaseling()) {
            if (this->siblings->connectLostSiblings() == CABINET_ERR) {
                logFatal("cluster cluster_id[%d] connect lost siblings error", this->getClusterId());
                exit(1);
            }
        }
        if (!this->children->satisfyWorkingBaseling()) {
            if (this->children->connectLostChildren() == CABINET_ERR) {
                logFatal("cluster cluster_id[%d] connect lost children error", this->getClusterId());
                exit(1);
            }
        }
        return CABINET_OK;
    }

    //do thing each role should do
    if (this->getClusterRole() == Cluster::LEADER) {
        //append entry to sibilings
        vector<ClusterClient *> onlineSiblings = this->siblings->getOnlineSiblings();
        Command &appendEntryCommand = this->commandKeeperPtr->selectCommand("appendentry");
        for (ClusterClient *sibling : onlineSiblings) {
            if ((appendEntryCommand >> sibling) == CABINET_ERR) {
                logWarning("cluster cluster_id[%d] append entry to sibling error", this->getClusterId());
                continue;
            }
        }

        //flush command to children
        ClusterClient *children = this->children->getOnlineChildren();
        Command &flushServerCommand = this->commandKeeperPtr->selectCommand("flushserver");
        if ((flushServerCommand >> children) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] flush server error", this->getClusterId());
        }
        return CABINET_OK;
    }

    if (this->getClusterRole() == Cluster::FOLLOWER) {
        long gap = this->updateTimeout();
        if (gap > this->electionTimeout) {
            logNotice("cluster cluster_id[%d] follower election timeout, change to candidate", this->getClusterId());
            if (this->toCandidate() == CABINET_ERR) {
                logFatal("cluster cluster_id[%d] to candidate error", this->getClusterId());
                exit(1);
            }
            return CABINET_OK;
        }
        return CABINET_OK;
    }

    if (this->getClusterRole() == Cluster::CANDIDATE) {
        long gap = this->updateTimeout();
        if (gap > this->electionTimeout) {
            //receive enough votes
            if (this->receiveVotes >= this->winVoteBaseline) {
                logNotice("cluster cluster_id[%d] candidate receive enough votes, change to leader", this->getClusterId());
                if (this->toLead() == CABINET_ERR) {
                    logFatal("cluster cluster_id[%d] to leader error", this->getClusterId());
                    exit(1);
                }
                return CABINET_OK;
            }

            //not enough votes
            logNotice("cluster cluster_id[%d] candidate election timeout, change to candidate", this->getClusterId());
            if (this->toCandidate() == CABINET_ERR) {
                logFatal("cluster cluster_id[%d] to candidate error", this->getClusterId());
                exit(1);
            }
            return CABINET_OK;
        }
        return CABINET_OK;
    }

    return CABINET_OK;
}

long Cluster::updateTimeout() {
    long currentUnixTimeInMs = Util::getCurrentTimeInMs();
    long gap = currentUnixTimeInMs - this->lastUnixTimeInMs;
    this->lastUnixTimeInMs = currentUnixTimeInMs;
    return gap;
}

int Cluster::nextCronTime() {
    return 1000 / this->hz;
}

/*
 *brief: send first empty appendEntry(heartbeat) to all siblings
 */
int Cluster::toLead() {
    logNotice("cluster cluster_id[%d] to lead", this->getClusterId());
    this->setClusterRole(Cluster::LEADER);
    this->votedFor = -1;

    vector<ClusterClient *> onlineSiblings = this->siblings->getOnlineSiblings();
    Command &firstAppendEntryCommand = this->commandKeeperPtr->selectCommand("appendentry");
    for (ClusterClient *sibling : onlineSiblings) {
        if ((firstAppendEntryCommand >> sibling) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] first append entry to sibling error", this->getClusterId());
            continue;
        }
    }

    Entry lastEntry;
    this->pf->findLastEntry(lastEntry);
    this->lastEntryIndex = lastEntry.getIndex();
    this->siblings->setNextIndexBatch(this->lastEntryIndex);
    this->siblings->setMatchIndexBatch(0);

    return CABINET_OK;
}

int Cluster::toFollow(long newTerm) {
    logNotice("cluster cluster_id[%d] to follow", this->getClusterId());
    this->setClusterRole(Cluster::FOLLOWER);

    this->currentTerm = newTerm;
    this->lastUnixTimeInMs = Util::getCurrentTimeInMs();
    this->votedFor = -1;
    
    return CABINET_OK;
}

int Cluster::toCandidate() {
    logNotice("cluster cluster_id[%d] to candidate", this->getClusterId());
    this->setClusterRole(Cluster::CANDIDATE);

    ++this->currentTerm;
    this->votedFor = this->getClusterId();
    this->receiveVotes = 1;
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

int Cluster::setNewEntryIndexAndTerm(Entry &entry) {
    entry.setIndex(this->lastEntryIndex);    
    entry.setTerm(this->currentTerm);
    return CABINET_OK;
}

Cluster::~Cluster()
{

}
