#include "Children.h"
#include "Const.h"

Children::Children():
    child(nullptr),
    connectStatus(false)
{
}

int Children::recognizeChildren(const Configuration &conf) {
    return CABINET_OK;
}

int Children::addChildren(ClusterClient *child) {
    return CABINET_OK;
}

int Children::deleteChildren(ClusterClient *child) {
    return CABINET_OK;
}

bool Children::satisfyWorkingBaseling() const {
    if (this->connectStatus == true) {
        return true;
    }
    return false;
}

int Children::connectLostChildren() {
    return CABINET_OK;
}

ClusterClient *Children::getOnlineChildren() const {
    if (this->connectStatus == true) {
        return this->child;
    }
    return nullptr;
}
