#include "Children.h"
#include "Const.h"
#include "Util.h"
#include <utility>
#include <algorithm>

Children::Children(Cluster *cluster):
    cluster(cluster),
    serverIdVector(),
    childPtrMap(),
    connectStatus(),
    ipMap(),
    portMap()
{
}

int Children::recognizeChildren(Configuration &conf) {
    logNotice("recognizing children");
    try{
        int childrenTotal = std::stoi(conf["CLUSTER_SERVER_TOTAL"]);
        for (int id = 1; id <= childrenTotal; ++id) {
            this->serverIdVector.push_back(id);
            this->childPtrMap[id] = nullptr;
            this->connectStatus[id] = false;
            this->ipMap[id] = conf[string("CLUSTER_SERVER_IP_") + std::to_string(id)];
            this->portMap[id] = std::stoi(conf[string("CLUSTER_SERVER_PORT_") + std::to_string(id)]);
            logNotice("cluster cluter_id[%d] get child IP[%s], port[%d]", this->cluster->getClusterId(), 
                    this->ipMap[id].c_str(), this->portMap[id]);
        }
    } catch (std::exception &e) {
        logFatal("children read conf fail, receive exception, what[%s]", e.what());
        return CABINET_ERR;
    }

    return CABINET_OK;
}

int Children::addChildren(int id, ClusterClient *child) {
    if (std::find(this->serverIdVector.begin(), this->serverIdVector.end(), id) == this->serverIdVector.end()) {
        logWarning("cluster[%d] add child but child not exist", this->cluster->getClusterId());
        return CABINET_ERR;
    }
    if (this->childPtrMap[id] != nullptr) {
        logWarning("cluster[%d] add child while child already exist", this->cluster->getClusterId());
        return CABINET_ERR;
    }
    this->childPtrMap[id] = child;
    this->connectStatus[id] = true;
    return CABINET_OK;
}

int Children::deleteChildren(ClusterClient *child) {
    int id = this->getChildrenId(child);
    if (id == CABINET_ERR) {
        logWarning("delete child, but child not exist");
        return CABINET_ERR;
    }
    if (this->childPtrMap[id] != child) {
        logWarning("delete child, but child not exist");
        return CABINET_ERR;
    }
    this->childPtrMap[id] = nullptr;
    this->connectStatus[id] = false;
    return CABINET_OK;
}

bool Children::satisfyWorkingBaseling() {
    bool result = true;
    for (int id : this->serverIdVector) {
        result &= this->connectStatus[id];
    }
    return result;
}

int Children::connectLostChildren() {
    int clusterId = this->cluster->getClusterId();
    logDebug("cluster cluster_id[%d] connect lost children", clusterId);

    if (this->satisfyWorkingBaseling() == true) {
        return CABINET_OK;
    }

    for (int id : this->serverIdVector) {
        if (this->connectStatus[id] == true) {
            logDebug("cluster cluster_id[%d] already connect No.%d children", clusterId, id);
            continue;
        }
        logDebug("cluster cluster_id[%d] has not connect No.%d children", clusterId, id);
        int connectFd;
        if ((connectFd = Util::connectTcp(this->ipMap[id].c_str(), this->portMap[id])) == CABINET_ERR) {
            logDebug("connect child error, ip[%s], port[%d]", this->ipMap[id].c_str(), this->portMap[id]);
            return CABINET_OK;
        }
        
        //connect success
        ClusterClient *newChild = this->cluster->createNormalClient(connectFd, this->ipMap[id], this->portMap[id]);
        newChild->useWrapProtocolStream();
        newChild->setCategory(Client::SERVER_CLIENT);
        EventPoll *eventpoll = this->cluster->getEventPoll();
        if (eventpoll->createFileEvent(newChild, READ_EVENT) == CABINET_ERR) {
            logWarning("add child into event poll error");
            delete newChild;
            return CABINET_OK;
        }
        if (this->addChildren(id, newChild) == CABINET_ERR) {
            logWarning("add child into children error");
            delete newChild;
            return CABINET_OK;
        }
        logNotice("cluster cluster_id[%d] connect No.%d lost children success", clusterId, id);
    }
    return CABINET_OK;
}

int Children::flushServer() {
    logNotice("cluster cluter_id[%d] flush server", this->cluster->getClusterId());
    return CABINET_OK;
}

int Children::shutDown() {
    for (int id : this->serverIdVector) {
        if (this->connectStatus[id] == false) {
            continue;
        }
        int connectFd = this->childPtrMap[id]->getClientFd();
        Util::closeConnectFd(connectFd);
    }
    return CABINET_OK;
}

int Children::getChildrenId(ClusterClient *child) {
    for (std::pair<int, ClusterClient *> childIter : this->childPtrMap) {
        if (childIter.second == child) {
            return childIter.first;
        }
    }
    return CABINET_ERR;
}
