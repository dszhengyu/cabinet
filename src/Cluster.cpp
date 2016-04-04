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
    pfName()
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
        this->electionTimeout = std::stoi(conf["CLUSTER_ELECTION_TIMEOUT_RATIO"]) * (1000 / this->hz);
        int clusterIDMax = std::stoi(conf["CLUSTER_ID_MAX"]);
        int clusterIDMin = std::stoi(conf["CLUSTER_ID_MIN"]);
        this->winVoteBaseline = (clusterIDMax - clusterIDMin + 1) / 2 + 1;
        this->pfName = conf["CLUSTER_PF_NAME"] + "." + conf["CLUSTER_ID"];
    } catch (std::exception &e) {
        logFatal("read conf fail, receive exception, what[%s]", e.what());
        exit(1);
    }

    logNotice("cluster cluster_id[%d] put into use", this->clusterId);

    this->siblings = new Siblings(this);
    if (this->siblings->recognizeSiblings(conf) == CABINET_ERR) {
        logFatal("cluster cluster_id[%d] recognize siblings error, exit", this->clusterId);
        exit(1);
    }

    this->children = new Children(this);
    if (this->children->recognizeChildren(conf) == CABINET_ERR) {
        logFatal("cluster cluster_id[%d] recognize children error, exit", this->clusterId);
        exit(1);
    }

    this->parents = new Parents(this);

    this->pf = new PersistenceFile(this->pfName, this->pfName + ".temp");
}

void Cluster::init() {
    if (Util::daemonize() == CABINET_ERR) {
        logFatal("cluster cluster_id[%d] daemonize fail, exit", this->clusterId);
        exit(1);
    }

    this->commandKeeperPtr = new CommandKeeper();
    this->commandKeeperPtr->createClusterCommandMap();

    while ((this->listenFd = this->listenOnPort(this->port)) == CABINET_ERR) {
        logFatal("cluster cluster_id[%d] listen on port[%d] error, keep trying", this->clusterId, this->port);
        sleep(5);
    }
   
    this->eventPoll = new EventPoll(this);
    if (this->eventPoll->initEventPoll() == CABINET_ERR) {
        logFatal("cluster cluster_id[%d] create event poll error", this->clusterId);
        exit(1);
    }

    if (this->eventPoll->pollListenFd(this->getListenFd()) == CABINET_ERR) {
        logFatal("cluster cluster_id[%d] pool listen fd error, port[%d]", this->clusterId, this->port);
        exit(1);
    }

    if (this->toFollow(1) == CABINET_ERR) {
        logFatal("cluster cluster_id[%d] init cluster cluster_id[%d] to follow error", this->clusterId);
        exit(1);
    }

    logNotice("init cluster cluster_id[%d] done", this->clusterId);
    #include "CabinetLogo.h"
    logNotice(cabinet_cluster_logo, this->port, this->pfName.c_str(), this->clusterId, this->hz, this->electionTimeout);
}

Client *Cluster::createClient(int listenFd) {
    logNotice("cluster cluster_id[%d] create new client, listen_fd[%d]", this->clusterId, listenFd);
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
        logNotice("cluster cluster_id[%d] add new client", this->clusterId);
        newClient = this->createNormalClient(connectFd, ip, port);
        if (this->parents->addParents(newClient) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] add cluster client to parents error", this->clusterId);
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
            logWarning("cluster cluster_id[%d] delete normal client error", this->clusterId);
            return CABINET_ERR;
        }
        logNotice("cluster cluster_id[%d] delete normal client", this->clusterId);
        return CABINET_OK;
    }
    if (client->getCategory() == Client::CLUSTER_CLIENT) {
        ClusterClient *clusterClient = (ClusterClient *)client;
        int siblingId = clusterClient->getClusterId();
        if (this->siblings->deleteSiblings(clusterClient) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] delete cluster[%d] error", this->clusterId, siblingId);
            return CABINET_ERR;
        }
        logNotice("cluster cluster_id[%d] delete cluster[%d]", this->clusterId, siblingId);
        return CABINET_OK;
    }
    if (client->getCategory() == Client::SERVER_CLIENT) {
        if (this->children->deleteChildren((ClusterClient *)client) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] delete server client error", this->clusterId);
            return CABINET_ERR;
        }
        logNotice("cluster cluster_id[%d] delete server client", this->clusterId);
        return CABINET_OK;
    }

    logFatal("cluster cluster_id[%d] can not delete unknown kind of client, broken, exit", this->clusterId);
    return CABINET_ERR;
}

/*
 *brief: 1. 首先检查siblings和children是否连接正常
 *          否, 则更改为follower状态, 连接服务器
 *       2. 根据不同角色做不同的事情
 */
int Cluster::cron() {
    logDebug("cluster cluster_id[%d] start cron, role[%c], term[%ld]", this->clusterId, this->role, this->currentTerm);
    //judge whether meet working baseline
    this->meetWorkingBaseline = this->siblings->satisfyWorkingBaseling() && this->children->satisfyWorkingBaseling();
    //reconnect if baseline not achieved
    if (!this->meetWorkingBaseline) {
        logWarning("cluster cluster_id[%d] could not meet working baseline", this->clusterId);
        if (!this->isFollower() && this->toFollow(this->currentTerm) == CABINET_ERR) {
            logFatal("cluster cluster_id[%d] change to follower error", this->clusterId);
            exit(1);
        }
        if (!this->siblings->satisfyWorkingBaseling()) {
            logWarning("cluster cluster_id[%d] could not meet working baseline, because of sibling", this->clusterId);
            if (this->siblings->connectLostSiblings() == CABINET_ERR) {
                logFatal("cluster cluster_id[%d] connect lost siblings error", this->clusterId);
                exit(1);
            }
        }
        if (!this->children->satisfyWorkingBaseling()) {
            logWarning("cluster cluster_id[%d] could not meet working baseline, because of children", this->clusterId);
            if (this->children->connectLostChildren() == CABINET_ERR) {
                logFatal("cluster cluster_id[%d] connect lost children error", this->clusterId);
                exit(1);
            }
        }
        this->meetWorkingBaseline = this->siblings->satisfyWorkingBaseling() && this->children->satisfyWorkingBaseling();
        if (this->meetWorkingBaseline) {
            logNotice("cluster cluster_id[%d] already meet working baseline", this->clusterId);
        }
        return CABINET_OK;
    }

    //flush command to children
    ClusterClient *children = this->children->getOnlineChildren();
    if (children != nullptr) {
        Command &flushServerCommand = this->commandKeeperPtr->selectCommand("flushserver");
        if ((flushServerCommand >> children) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] flush server error", this->clusterId);
        }
    }

    //do thing each role should do
    if (this->isLeader()) {
        logDebug("cluster cluster_id[%d] work as leader, term[%ld]", this->clusterId, this->currentTerm);
        //append entry to sibilings
        vector<ClusterClient *> onlineSiblings = this->siblings->getSiblingsNeedAppendEntry();
        Command &appendEntryCommand = this->commandKeeperPtr->selectCommand("appendentry");
        for (ClusterClient *sibling : onlineSiblings) {
            if ((appendEntryCommand >> sibling) == CABINET_ERR) {
                logWarning("cluster cluster_id[%d] append entry to sibling error", this->clusterId);
                if (!this->isLeader()) {
                    return CABINET_OK;
                }
                continue;
            }
        }

        return CABINET_OK;
    }

    if (this->isFollower()) {
        logDebug("cluster cluster_id[%d] work as follower, term[%ld]", this->clusterId, this->currentTerm);
        long currentUnixTimeInMs = Util::getCurrentTimeInMs();
        long gap = currentUnixTimeInMs - this->lastUnixTimeInMs;
        if (gap > this->electionTimeout) {
            logNotice("cluster cluster_id[%d] follower election timeout, change to candidate", this->clusterId);
            if (this->toCandidate() == CABINET_ERR) {
                logFatal("cluster cluster_id[%d] to candidate error", this->clusterId);
                exit(1);
            }
            return CABINET_OK;
        }
        return CABINET_OK;
    }

    if (this->isCandidate()) {
        logDebug("cluster cluster_id[%d] work as candidate, term[%ld]", this->clusterId, this->currentTerm);
        long currentUnixTimeInMs = Util::getCurrentTimeInMs();
        long gap = currentUnixTimeInMs - this->lastUnixTimeInMs;
        if (gap > this->electionTimeout) {
            //receive enough votes
            if (this->receiveVotes >= this->winVoteBaseline) {
                logNotice("cluster cluster_id[%d] candidate receive enough votes, change to leader", this->clusterId);
                if (this->toLead() == CABINET_ERR) {
                    logFatal("cluster cluster_id[%d] to leader error", this->clusterId);
                    exit(1);
                }
                return CABINET_OK;
            }

            //not enough votes
            logNotice("cluster cluster_id[%d] candidate election timeout, change to candidate", this->clusterId);
            int sleepAWhile = this->electionTimeout / this->clusterId;
            logDebug("cluster cluster_id[%d] candidate election timeout, randomly sleep for a while, [%d]ms",
                    this->clusterId, sleepAWhile);
            usleep(sleepAWhile * 1000);
            if (this->toCandidate() == CABINET_ERR) {
                logFatal("cluster cluster_id[%d] to candidate error", this->clusterId);
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
    logNotice("cluster cluster_id[%d] to lead", this->clusterId);
    this->setClusterRole(Cluster::LEADER);
    this->votedFor = -1;

    Entry lastEntry;
    this->pf->findLastEntry(lastEntry);
    this->lastEntryIndex = lastEntry.getIndex();
    long nextIndex = this->lastEntryIndex + 1;
    this->siblings->setNextIndexBatch(nextIndex);
    this->siblings->setMatchIndexBatch(0);
    this->siblings->setAlreadyAppendEntryBatch(false);

    return CABINET_OK;
}

int Cluster::toFollow(long newTerm) {
    logNotice("cluster cluster_id[%d] to follow, new_term[%d]", this->clusterId, newTerm);
    this->setClusterRole(Cluster::FOLLOWER);

    this->currentTerm = newTerm;
    this->lastUnixTimeInMs = Util::getCurrentTimeInMs();
    this->votedFor = -1;
    
    return CABINET_OK;
}

int Cluster::toCandidate() {
    logNotice("cluster cluster_id[%d] to candidate", this->clusterId);
    this->setClusterRole(Cluster::CANDIDATE);

    ++this->currentTerm;
    this->votedFor = this->clusterId;
    this->receiveVotes = 1;
    this->lastUnixTimeInMs = Util::getCurrentTimeInMs();

    vector<ClusterClient *> onlineSiblings = this->siblings->getSiblingsNeedAppendEntry();
    Command &requestVoteCommand = this->commandKeeperPtr->selectCommand("requestvote");
    for (ClusterClient *sibling : onlineSiblings) {
        if ((requestVoteCommand >> sibling) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] request vote from sibling error", this->clusterId);
            continue;
        }
    }

    return CABINET_OK;
}

int Cluster::setNewEntryIndexAndTerm(Entry &entry) {
    entry.setIndex(++this->lastEntryIndex);    
    entry.setTerm(this->currentTerm);
    return CABINET_OK;
}

int Cluster::shutDown() {
    logFatal("cluster cluster_id[%d] shut down ing", this->clusterId);
    this->siblings->shutDown();
    this->children->shutDown();
    this->parents->shutDown();
    return CABINET_OK;
}

Cluster::~Cluster()
{

}
