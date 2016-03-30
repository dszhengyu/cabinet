#include "Siblings.h"
#include "Const.h"
#include "Util.h"

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
    ipPortClusterIdMap()
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
            int port = std::stoi(conf[string("CLUSTER_CLUSTER_PORT_") + std::to_string(idReading)]);
            this->ipMap[idReading] = ip;
            this->portMap[idReading] = port;
            this->nextIndexMap[idReading] = 0;
            this->matchIndexMap[idReading] = 0;
            this->connectStatus[idReading] = false;
            this->clusterIdClientPtrMap[idReading] = nullptr;
            string ipPort = ip + ":" + std::to_string(port);
            this->ipPortClusterIdMap[ipPort] = idReading;
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
    logNotice("cluster cluster_id[%d] receive sibling connection, sibling_ip[%s], sibling_port[%d]", 
            this->clusterId, sibling->getIp().c_str(), sibling->getPort());
    int clusterId = this->getSiblingClusterId(sibling);
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
    this->connectStatus[clusterId] = true;
    this->nextIndexMap[clusterId] = 0;
    this->matchIndexMap[clusterId] = 0;
    logNotice("cluster cluster_id[%d] validate sibling connection, sibling_cluster_id[%d], sibling_ip[%s], sibling_port[%d]", 
            this->clusterId, clusterId, sibling->getIp().c_str(), sibling->getPort());

    return CABINET_OK;
}

int Siblings::deleteSiblings(ClusterClient *sibling) {
    int clusterId = sibling->getClusterId();
    if (this->clusterIdClientPtrMap.find(clusterId) == this->clusterIdClientPtrMap.end()) {
        logWarning("cluster cluster_id[%d] trying to delete cluster with invalid id, id[%d]", this->clusterId, clusterId);
        return CABINET_ERR;
    }

    if (this->clusterIdClientPtrMap[clusterId] == nullptr) {
        logFatal("cluster cluster_id[%d] trying to delete cluster while it is already deleted, id[%d]", this->clusterId, clusterId);
        return CABINET_ERR;
    }

    this->clusterIdClientPtrMap[clusterId] = nullptr;
    this->connectStatus[clusterId] = false;
    this->nextIndexMap[clusterId] = 0;
    this->matchIndexMap[clusterId] = 0;

    return CABINET_OK;
}

vector<ClusterClient *> Siblings::getOnlineSiblings() {
    vector<ClusterClient *> siblingsVector;
    for (int clusterId : this->clusterIdVector) {
        if (this->connectStatus[clusterId] == true) {
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
            int connectFd;
            if ((connectFd = Util::connectTcp(ipMap[clusterId].c_str(), portMap[clusterId])) == CABINET_ERR) {
                logWarning("cluster cluster_id[%d] connect No.%d siblings error", this->clusterId, clusterId);
                continue;
            }
            //connect success
            ClusterClient *newSibling = this->cluster->createNormalClient(connectFd, ipMap[clusterId], portMap[clusterId]);
            newSibling->setCategory(Client::CLUSTER_CLIENT);
            EventPoll *eventpoll = this->cluster->getEventPoll();
            if (eventpoll->createFileEvent(newSibling, READ_EVENT) == CABINET_ERR) {
                logWarning("cluster cluster_id[%d] add sibling into event poll error", this->clusterId);
                delete newSibling;
                continue;
            }
            if (this->addSiblings(newSibling) == CABINET_ERR) {
                logWarning("cluster cluster_id[%d] add sibling into sinblings error", this->clusterId);
                delete newSibling;
                continue;
            }
            logNotice("cluster cluster_id[%d] connect sibling[%d} success", this->clusterId, clusterId);
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
