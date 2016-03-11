#include "Siblings.h"
#include "Const.h"
#include "Util.h"

Siblings::Siblings():
    clusterId(-1),
    clusterIdVector(),
    ipMap(),
    portMap(),
    nextIndexMap(),
    matchIndexMap(),
    connectStatus(),
    clusterIdClientPtrMap()
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
            logNotice("recognize No.%d siblings, ip[%s], port[%d]", idReading, ip.c_str(), port);
        }
    } catch (std::exception &e) {
        logFatal("siblings read conf fail, receive exception, what[%s]", e.what());
        return CABINET_ERR;
    }

    return CABINET_OK;
}

int Siblings::addSiblings(int clusterId, ClusterClient *sibling) {
    if (this->clusterIdClientPtrMap.find(clusterId) == this->clusterIdClientPtrMap.end()) {
        logWarning("trying to add cluster with invalid id, id[%d]", clusterId);
        return CABINET_ERR;
    }

    if (this->connectStatus[clusterId] == true) {
        logWarning("trying to add cluster while it is already connected, id[%d]", clusterId);
        return CABINET_ERR;
    }

    if (this->clusterIdClientPtrMap[clusterId] != nullptr) {
        logWarning("trying to add cluster while it is already registered, id[%d]", clusterId);
        return CABINET_ERR;
    }

    this->clusterIdClientPtrMap[clusterId] = sibling;
    this->connectStatus[clusterId] = true;
    this->nextIndexMap[clusterId] = 0;
    this->matchIndexMap[clusterId] = 0;

    return CABINET_OK;
}

int Siblings::deleteSiblings(ClusterClient *sibling) {
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
 */
int Siblings::connectLostSiblings() {
    for (int clusterId : this->clusterIdVector) {
        if (clusterId >= this->clusterId) {
            continue;
        }

        if (this->connectStatus[clusterId] == false) {
            int connectFd;
            if ((connectFd = Util::connectTcp(ipMap[clusterId].c_str(), portMap[clusterId])) == CABINET_ERR) {
                logWarning("connect No.%d siblings error", clusterId);
                continue;
            }
            //connect success
        }
    }
    return CABINET_OK;
}
