#include "Children.h"
#include "Const.h"
#include "Util.h"
#include <utility>
#include <algorithm>
#include <utility>

Children::Children(Cluster *cluster):
    cluster(cluster),
    serverIdVector(),
    childPtrMap(),
    connectStatus(),
    ipMap(),
    portMap(),
    consistentHashAndServerIdMap(),
    hashValueMax(977)
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
    //add into consistent hash
    long hashUnit = this->hashValueMax / this->serverIdVector.size();
    long hashValue = hashUnit * id;
    this->consistentHashAndServerIdMap[hashValue] = id;
    logNotice("cluster cluster_id[%d] add children[%d] hash_value[%ld] hash_max_value[%d]",
            this->cluster->getClusterId(), id, hashValue, this->hashValueMax);
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

    //delete from consistent hash
    auto mapIt = this->consistentHashAndServerIdMap.begin();
    for (; mapIt != this->consistentHashAndServerIdMap.end(); ++mapIt) {
        if ((*mapIt).second == id) {
            break;
        }
    }
    if (mapIt == this->consistentHashAndServerIdMap.end()) {
        logWarning("cluster cluster_id[%d] have not add server[%d] into consistent hash", this->cluster->getClusterId(), id);
        return CABINET_OK;
    }
    this->consistentHashAndServerIdMap.erase(mapIt);

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
    Cluster *cluster = this->cluster;
    int clusterId = cluster->getClusterId();
    logDebug("cluster cluster_id[%d] check if need to flush command to server", clusterId);

    //确定应该发送的entry
    long lastApplied = cluster->getLastApplied();
    long commitIndex = cluster->getIndex();

    if (commitIndex == lastApplied) {
        logDebug("cluster cluster_id[%d] not need to flush command to server", clusterId);
        return CABINET_OK; 
    }

    if (commitIndex < lastApplied) {
        logFatal("in cluster[%d] something wrong with commitIndex and lastApplied, program fail", clusterId);
        exit(1);
    }

    //获取应该发送的enty
    cluster->increaseLastApplied();
    long sendingEntryIndex = cluster->getLastApplied();
    logDebug("cluster cluster_id[%d] flush command to server, index[%ld]", clusterId, sendingEntryIndex);
    PersistenceFile *pf = cluster->getPersistenceFile();
    Entry sendingEntry;
    if (pf->findEntry(sendingEntryIndex, sendingEntry) == CABINET_ERR) {
        logWarning("cluster cluster_id[%d] could not find the entry to flush to server", clusterId);
        return CABINET_ERR;
    }

    //发送entry
    const string &content = sendingEntry.getContent();
    ClusterClient *respondServer = this->getRespondServer(content);
    if (respondServer == nullptr) {
        logWarning("cluster cluster[%d] find respond server to flush command error", clusterId);
        return CABINET_ERR;
    }
    respondServer->fillSendBuf(content);
    respondServer->printSendBuf();
    respondServer->getReadyToSendMessage();

    return CABINET_OK;
}

ClusterClient *Children::getRespondServer(const string &content) {
    //resolve content to get key
    ProtocolStream pt(true);
    pt.fillReceiveBuf(content);
    if (pt.resolveReceiveBuf() == CABINET_ERR) {
        logWarning("resolve command error, command_content[%s]", content.c_str());
        return nullptr;
    }
    const vector<string> &inputArgv = pt.getReceiveArgv();
    string strToHash;
    if (inputArgv.size() == 1) {
        strToHash = inputArgv[0]; 
    }
    else {
        strToHash = inputArgv[1];
    }
    //hash
    std::hash<string> strHasher;
    int hashValue = strHasher(strToHash) % this->hashValueMax;

    //find the server to return
    if (this->consistentHashAndServerIdMap.empty()) {
        logWarning("cluster[%d] have none server in consistent hash", this->cluster->getClusterId());
        return nullptr;
    }
    int serverId = -1;
    for (std::pair<long, int> hashServerIdPair : this->consistentHashAndServerIdMap) {
        logDebug("flush server, judge server[%d], hash_value[%ld]", hashServerIdPair.second, hashServerIdPair.first);
        if (hashValue <= hashServerIdPair.first) {
            serverId = hashServerIdPair.second;
            break;
        }
    }
    if (serverId == -1) {
        serverId = (*this->consistentHashAndServerIdMap.begin()).second;
    }

    //check server and return
    if (this->connectStatus[serverId] == false) {
        logFatal("server lost connection but still exist in consistent hash");
        exit(1);
    }
    logNotice("cluster cluster[%d] flush command to server[%d], str_to_hash[%s], hash_value[%d]", 
            this->cluster->getClusterId(), serverId, strToHash.c_str(), hashValue);
    return this->childPtrMap[serverId];
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
