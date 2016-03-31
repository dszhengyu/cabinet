#include "Parents.h"
#include "Const.h"
#include "Util.h"
#include <algorithm>
using std::find;

Parents::Parents(Cluster *cluster):
    parentsQueue(),
    dealingIndexParentsMap(),
    cluster(cluster)
{
}

int Parents::addParents(ClusterClient *parents) {
    logDebug("cluster cluster_id[%d] add parents", this->cluster->getClusterId());
    if (this->findParentsInQueue(parents) != this->parentsQueue.end()) {
        logWarning("cluster cluster_id[%d] add parents which it is already exist", this->cluster->getClusterId());
        return CABINET_ERR;
    }
    this->parentsQueue.push_back(parents);
    return CABINET_OK;
}

int Parents::deleteParents(ClusterClient *parents) {
    logDebug("cluster cluster_id[%d] delete parents", this->cluster->getClusterId());
    Parents::queueIter findResult = this->findParentsInQueue(parents);
    if (findResult == this->parentsQueue.end()) {
        logWarning("cluster cluster_id[%d] delete parents which it is not exist", this->cluster->getClusterId());
        return CABINET_ERR;
    }
    this->parentsQueue.erase(findResult);
    return CABINET_OK;
}

Parents::queueIter Parents::findParentsInQueue(ClusterClient *parents) { 
    Parents::queueIter findResult;
    if ((findResult = find(this->parentsQueue.begin(), this->parentsQueue.end(), parents)) 
            == this->parentsQueue.end()) {
        //logDebug("no find match parents");
    }
    else {
        //logDebug("parents exist");
    }
    return findResult;
}

int Parents::setDealingIndex(long dealingIndex, ClusterClient *parent) {
    logDebug("cluster cluster_id[%d] set dealing index in parents", this->cluster->getClusterId());
    if (this->findParentsInQueue(parent) == this->parentsQueue.end()) {
        logWarning("cluster cluster_id[%d] set dealing index to no-exist parents", this->cluster->getClusterId());
        return CABINET_ERR;
    }

    this->dealingIndexParentsMap[dealingIndex] = parent;
    return CABINET_OK; 
}

ClusterClient *Parents::getParentsByDealingIndex(long dealingIndex) {
   if (this->dealingIndexParentsMap.find(dealingIndex) == this->dealingIndexParentsMap.end()) {
        return nullptr;
   }
   ClusterClient *parent = this->dealingIndexParentsMap[dealingIndex];
   this->dealingIndexParentsMap.erase(dealingIndex);
   return parent;
}

int Parents::shutDown() {
    for (ClusterClient *parent : this->parentsQueue) {
        int connectFd = parent->getClientFd();
        Util::closeConnectFd(connectFd);
    }
    return CABINET_OK;
}
