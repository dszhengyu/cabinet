#include "Siblings.h"
#include "Const.h"

Siblings::Siblings()
{
}

int Siblings::recognizeSiblings(const Configuration &conf) {
    return CABINET_OK;
}

int Siblings::addSiblings(ClusterClient *sibling) {
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

int Siblings::connectLostSiblings() {
    return CABINET_OK;
}
