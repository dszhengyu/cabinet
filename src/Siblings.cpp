#include "Siblings.h"
#include "Const.h"
#include "Util.h"
#include "CommandKeeper.h"

Siblings::Siblings(Cluster *cluster):
    clusterId(-1),
    clusterIdVector(),
    ipMap(),
    portMap(),
    nextIndexMap(),
    matchIndexMap(),
    connectStatus(),
    clusterIdClientPtrMap(),
    cluster(cluster),
    currentLeaderId(-1),
    ipPortClusterIdMap(),
    connectTrying(),
    alreadyAppendEntry(),
    emptyAppendEntry()
{
}

int Siblings::recognizeSiblings(Configuration &conf) {
    logNotice("recognizing siblings");
    int idMin;
    int idMax;
    try{
        this->clusterId = std::stoi(conf["CLUSTER_ID"]);
        idMin = std::stoi(conf["CLUSTER_ID_MIN"]);
        idMax = std::stoi(conf["CLUSTER_ID_MAX"]);
        if (idMin < 1) {
            logFatal("cluster min id should not less than 1, please re-configuration");
            exit(1);
        }
        for (int idReading = idMin; idReading <= idMax; ++idReading) {
            if (idReading == this->clusterId) {
                continue;
            }
            clusterIdVector.push_back(idReading);
            string ip = conf[string("CLUSTER_IP_") + std::to_string(idReading)];
            int port = std::stoi(conf[string("CLUSTER_PORT_") + std::to_string(idReading)]);
            this->ipMap[idReading] = ip;
            this->portMap[idReading] = port;
            this->nextIndexMap[idReading] = 0;
            this->matchIndexMap[idReading] = 0;
            this->connectStatus[idReading] = false;
            this->clusterIdClientPtrMap[idReading] = nullptr;
            string ipPort = ip + ":" + std::to_string(port);
            this->ipPortClusterIdMap[ipPort] = idReading;
            this->connectTrying[idReading] = false;
            this->alreadyAppendEntry[idReading] = false;
            logNotice("cluster cluster_id[%d] recognize No.%d siblings, ip[%s], port[%d]", 
                    this->clusterId, idReading, ip.c_str(), port);
        }
    } catch (std::exception &e) {
        logFatal("cluster cluster_id[%d] siblings read conf fail, receive exception, what[%s]", this->clusterId, e.what());
        return CABINET_ERR;
    }

    return CABINET_OK;
}

int Siblings::getSiblingClusterId(ClusterClient *sibling) {
    const string &ip = sibling->getIp();
    const int port = sibling->getPort();
    string ipPort = ip + ":" + std::to_string(port);
    if (this->ipPortClusterIdMap.find(ipPort) == this->ipPortClusterIdMap.end()) {
        logWarning("cluster cluster_id[%d] receive connecting from cluster port, but not a sibling. ip[%s], port[%d]",
                this->clusterId, ip.c_str(), port);
        return CABINET_ERR;
    }
    return this->ipPortClusterIdMap[ipPort];
}

int Siblings::addSiblings(ClusterClient *sibling) {
    logNotice("cluster cluster_id[%d] add sibling, sibling_ip[%s], sibling_port[%d]", 
            this->clusterId, sibling->getIp().c_str(), sibling->getPort());
    int clusterId = sibling->getClusterId();
    if (clusterId == CABINET_ERR) {
        logWarning("cluster cluster_id[%d] trying to add cluster which is outside conf", this->clusterId);
        return CABINET_ERR;
    }
    if (this->clusterIdClientPtrMap.find(clusterId) == this->clusterIdClientPtrMap.end()) {
        logWarning("cluster cluster_id[%d] trying to add cluster with invalid id, id[%d]", this->clusterId, clusterId);
        return CABINET_ERR;
    }

    if (this->connectStatus[clusterId] == true) {
        logWarning("cluster cluster_id[%d] trying to add cluster while it is already connected, id[%d]", this->clusterId, clusterId);
        return CABINET_ERR;
    }

    if (this->clusterIdClientPtrMap[clusterId] != nullptr) {
        logWarning("cluster cluster_id[%d] trying to add cluster while it is already registered, id[%d]", this->clusterId, clusterId);
        return CABINET_ERR;
    }

    this->clusterIdClientPtrMap[clusterId] = sibling;
    this->connectStatus[clusterId] = false;
    this->nextIndexMap[clusterId] = 0;
    this->matchIndexMap[clusterId] = 0;
    this->connectTrying[clusterId] = false;
    this->alreadyAppendEntry[clusterId] = false;
    logNotice("cluster cluster_id[%d] validate sibling, sibling_cluster_id[%d], sibling_ip[%s], sibling_port[%d]", 
            this->clusterId, clusterId, sibling->getIp().c_str(), sibling->getPort());

    return CABINET_OK;
}

int Siblings::deleteSiblings(ClusterClient *sibling) {
    int clusterId = sibling->getClusterId();
    logDebug("cluster cluster_id[%d] trying to delete cluster[%d]", this->clusterId, clusterId);
    if (this->clusterIdClientPtrMap.find(clusterId) == this->clusterIdClientPtrMap.end()) {
        logWarning("cluster cluster_id[%d] trying to delete cluster with invalid id, cluster_id[%d]", this->clusterId, clusterId);
        return CABINET_ERR;
    }

    if (this->clusterIdClientPtrMap[clusterId] == nullptr) {
        logFatal("cluster cluster_id[%d] trying to delete cluster while it is already deleted, cluster_id[%d]", 
                this->clusterId, clusterId);
        return CABINET_ERR;
    }

    this->clusterIdClientPtrMap[clusterId] = nullptr;
    this->connectStatus[clusterId] = false;
    this->nextIndexMap[clusterId] = 0;
    this->matchIndexMap[clusterId] = 0;
    this->connectTrying[clusterId] = false;
    this->alreadyAppendEntry[clusterId] = false;

    return CABINET_OK;
}

vector<ClusterClient *> Siblings::getSiblingsNeedAppendEntry() {
    vector<ClusterClient *> siblingsVector;
    for (int clusterId : this->clusterIdVector) {
        if (this->connectStatus[clusterId] == true &&
                this->alreadyAppendEntry[clusterId] == false) {
            siblingsVector.push_back(this->clusterIdClientPtrMap[clusterId]);
            continue;
        }
    }
    return siblingsVector;
}

bool Siblings::satisfyWorkingBaseling() {
    bool finalVal = true;
    for (int clusterId : this->clusterIdVector) {
        finalVal &= this->connectStatus[clusterId];
    }
    return finalVal;
}

/*
 *brief: only connect lost siblings whose id is smaller than "this"
 *step: 1. get connect
 *      2. create clusterclient with correct tyoe
 *      3. add the new client into siblings
 *      4. add the new client into eventpoll
 */
int Siblings::connectLostSiblings() {
    int cabinetClusterId = this->cluster->getClusterId();
    logDebug("cluster cluster_id[%d] connect lost siblings", cabinetClusterId);
    for (int clusterId : this->clusterIdVector) {
        if (this->connectStatus[clusterId] == true) {
            logDebug("cluster cluster_id[%d] already connect No.%d sibling", this->clusterId, clusterId);
        }
        else {
            logDebug("cluster cluster_id[%d] have not connect No.%d sibling", this->clusterId, clusterId);
        }
        if (clusterId >= this->clusterId) {
            continue;
        }

        if (this->connectStatus[clusterId] == false) {
            if (this->connectTrying[clusterId] == true) {
                logDebug("cluster cluster_id[%d] connect No.%d sibling processing, waiting for reply", this->clusterId, clusterId);
                continue;
            }

            int connectFd;
            if ((connectFd = Util::connectTcp(ipMap[clusterId].c_str(), portMap[clusterId])) == CABINET_ERR) {
                logWarning("cluster cluster_id[%d] connect No.%d siblings error", this->clusterId, clusterId);
                continue;
            }
            //connect success
            ClusterClient *newSibling = this->cluster->createNormalClient(connectFd, ipMap[clusterId], portMap[clusterId]);
            newSibling->setClusterId(clusterId);
            newSibling->setCategory(Client::CLUSTER_CLIENT);
            EventPoll *eventpoll = this->cluster->getEventPoll();
            if (eventpoll->createFileEvent(newSibling, READ_EVENT) == CABINET_ERR) {
                logWarning("cluster cluster_id[%d] add sibling into event poll error", this->clusterId);
                delete newSibling;
                continue;
            }
            if (this->addSiblings(newSibling) == CABINET_ERR) {
                logWarning("cluster cluster_id[%d] add sibling into siblings error", this->clusterId);
                eventpoll->removeFileEvent(newSibling, READ_EVENT);
                delete newSibling;
                continue;
            }
            logNotice("cluster cluster_id[%d] connect sibling[%d] half success, sending cluster node message", this->clusterId, clusterId);
            CommandKeeper *commandKeeperPtr = this->cluster->getCommandKeeper();
            Command &clusterNodeCommand = commandKeeperPtr->selectCommand("clusternode");
            if ((clusterNodeCommand >> newSibling) == CABINET_ERR) {
                logWarning("cluster cluster_id[%d] send sibling cluster node message error", this->clusterId);
                eventpoll->removeFileEvent(newSibling, READ_EVENT);
                this->deleteSiblings(newSibling);
                delete newSibling;
                continue;
            }
            this->connectTrying[clusterId] = true;
        }
    }
    return CABINET_OK;
}

long Siblings::getSiblingNextIndex(int clusterId) {
    if (this->validateClusterId(clusterId) == CABINET_ERR) {
        logWarning("get sibling next index error with invalid id, id[%d]", clusterId);
        return CABINET_ERR;
    }
    return this->nextIndexMap[clusterId];
}

int Siblings::increaseSiblingNextIndex(int clusterId) {
    if (this->validateClusterId(clusterId) == CABINET_ERR) {
        logWarning("get sibling next index error with invalid id, id[%d]", clusterId);
        return CABINET_ERR;
    }
    ++this->nextIndexMap[clusterId];
    return CABINET_OK;
}

int Siblings::decreaseSiblingNextIndex(int clusterId) {
    if (this->validateClusterId(clusterId) == CABINET_ERR) {
        logWarning("get sibling next index error with invalid id, id[%d]", clusterId);
        return CABINET_ERR;
    }
    --this->matchIndexMap[clusterId];
    return CABINET_OK;
}

int Siblings::validateClusterId(int clusterId) {
    if (this->clusterIdClientPtrMap.find(clusterId) == this->clusterIdClientPtrMap.end()) {
        return CABINET_ERR;
    }
    return CABINET_OK;
}

void Siblings::setNextIndexBatch(long newNextIndex) {
    for (int clusterId : this->clusterIdVector) {
        this->nextIndexMap[clusterId] = newNextIndex;
    }
}

void Siblings::setMatchIndexBatch(long newMatchIndex) {
    for (int clusterId : this->clusterIdVector) {
        this->matchIndexMap[clusterId] = newMatchIndex;
    }
}

void Siblings::setMatchIndex(int clusterId, long newMatchIndex) {
    if (this->validateClusterId(clusterId) == CABINET_ERR) {
        logWarning("get sibling next index error with invalid id, id[%d]", clusterId);
        return;
    }
    this->matchIndexMap[clusterId] = newMatchIndex;
}

int Siblings::getLeaderIPAndPort(string &ip, int &port) {
    ip = this->ipMap[this->currentLeaderId];
    port = this->portMap[this->currentLeaderId];
    return CABINET_OK;
}

int Siblings::confirmConnectSibling(ClusterClient *sibling) {
    int siblingId = sibling->getClusterId();
    logDebug("cluster cluster_id[%d] confirm connect No.%d sibling", this->clusterId, siblingId);
    if (this->validateClusterId(siblingId) == CABINET_ERR) {
        logWarning("get sibling confirm sibling connection with invalid id, id[%d]", clusterId);
        return CABINET_ERR;
    }
    this->connectStatus[siblingId] = true;
    this->connectTrying[siblingId] = false;
    return CABINET_OK;
}

int Siblings::shutDown() {
    for (int clusterId : this->clusterIdVector) {
        if (this->connectStatus[clusterId] == false) {
            continue;
        }
        int connectFd = this->clusterIdClientPtrMap[clusterId]->getClientFd();
        Util::closeConnectFd(connectFd);
    }
    return CABINET_OK;
}

int Siblings::setAlreadyAppendEntry(int clusterId, bool already) {
    //logDebug("cluster cluster_id[%d] set cluster[%d] alreadyAppendEntry[%s]", this->clusterId, clusterId,
    //        (already ? "true" : "false"));
    if (this->validateClusterId(clusterId) == CABINET_ERR) {
        logWarning("set already append entry error with invalid id, id[%d]", clusterId);
        return CABINET_ERR;
    }
    this->alreadyAppendEntry[clusterId] = already;
    return CABINET_OK;
}

void Siblings::setAlreadyAppendEntryBatch(bool alreadyBatch) {
    for (int clusterId : this->clusterIdVector) {
        this->alreadyAppendEntry[clusterId] = alreadyBatch;
    }
}

int Siblings::setEmptyAppendEntry(int clusterId, bool empty) {
    logDebug("cluster cluster_id[%d] set cluster[%d] emptyAppendEntry[%s]", this->clusterId, clusterId,
            (empty ? "true" : "false"));
    if (this->validateClusterId(clusterId) == CABINET_ERR) {
        logWarning("set empty append entry error with invalid id, id[%d]", clusterId);
        return CABINET_ERR;
    }
    this->emptyAppendEntry[clusterId] = empty;
    return CABINET_OK;
}

int Siblings::getEmptyAppendEntry(const int clusterId, bool &empty) {
    logDebug("cluster cluster_id[%d] get cluster[%d] emptyAppendEntry", this->clusterId, clusterId);
    if (this->validateClusterId(clusterId) == CABINET_ERR) {
        logWarning("get empty append entry error with invalid id, id[%d]", clusterId);
        return CABINET_ERR;
    }
    empty = this->emptyAppendEntry[clusterId];
    return CABINET_OK;
}

