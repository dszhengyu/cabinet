#include "Parents.h"
#include "Const.h"
#include <algorithm>
using std::find;

Parents::Parents():
    parentsQueue()
{
}

int Parents::addParents(ClusterClient *parents) {
    logDebug("add parents");
    if (this->findParentsInQueue(parents) != this->parentsQueue.end()) {
        logWarning("add parents which it is already exist");
        return CABINET_ERR;
    }
    this->parentsQueue.push_back(parents);
    return CABINET_OK;
}

int Parents::deleteParents(ClusterClient *parents) {
    logDebug("delete parents");
    if (this->findParentsInQueue(parents) == this->parentsQueue.end()) {
        logWarning("delete parents which it is not exist");
        return CABINET_ERR;
    }
    this->parentsQueue.push_back(parents);
    return CABINET_OK;
}

Parents::queueIter Parents::findParentsInQueue(ClusterClient *parents) { 
    Parents::queueIter findResult;
    if ((findResult = find(this->parentsQueue.begin(), this->parentsQueue.end(), parents)) 
            == this->parentsQueue.end()) {
        logDebug("no find match parents");
    }
    logDebug("parents exist");
    return findResult;
}
