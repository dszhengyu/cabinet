#include "Children.h"
#include "Const.h"
#include "Util.h"

Children::Children(Cluster *cluster):
    cluster(cluster),
    child(nullptr),
    connectStatus(false),
    ip(),
    port(-1)
{
}

int Children::recognizeChildren(Configuration &conf) {
    logNotice("recognizing children");
    try{
        int childrenTotal = std::stoi(conf["CLUSTER_SERVER_TOTAL"]);
        for (int id = 1; id <= childrenTotal; ++id) {
            this->ip = conf[string("CLUSTER_SERVER_IP_") + std::to_string(id)];
            this->port = std::stoi(conf[string("CLUSTER_SERVER_PORT_") + std::to_string(id)]);
            logNotice("cluster cluter_id[%d] get child IP[%s], port[%d]", this->cluster->getClusterId(), this->ip.c_str(), this->port);
        }
    } catch (std::exception &e) {
        logFatal("children read conf fail, receive exception, what[%s]", e.what());
        return CABINET_ERR;
    }

    return CABINET_OK;
}

int Children::addChildren(ClusterClient *child) {
    if (this->child != nullptr) {
        logWarning("add child while child already exist");
        return CABINET_ERR;
    }
    this->child = child;
    this->connectStatus = true;
    return CABINET_OK;
}

int Children::deleteChildren(ClusterClient *child) {
    if (this->child != child) {
        logWarning("delete child, but child not exist");
        return CABINET_ERR;
    }
    this->child = nullptr;
    this->connectStatus = false;
    return CABINET_OK;
}

bool Children::satisfyWorkingBaseling() const {
    if (this->connectStatus == true) {
        return true;
    }
    return false;
}

int Children::connectLostChildren() {
    int clusterId = this->cluster->getClusterId();
    logDebug("cluster cluster_id[%d] connect lost children", clusterId);

    if (this->satisfyWorkingBaseling() == true) {
        return CABINET_OK;
    }

    int connectFd;
    if ((connectFd = Util::connectTcp(this->ip.c_str(), this->port)) == CABINET_ERR) {
        logDebug("connect child error, ip[%s], port[%d]", this->ip.c_str(), this->port);
        return CABINET_OK;
    }
    
    //connect success
    ClusterClient *newChild = this->cluster->createNormalClient(connectFd, this->ip, this->port);
    newChild->useWrapProtocolStream();
    newChild->setCategory(Client::SERVER_CLIENT);
    EventPoll *eventpoll = this->cluster->getEventPoll();
    if (eventpoll->createFileEvent(newChild, READ_EVENT) == CABINET_ERR) {
        logWarning("add child into event poll error");
        delete newChild;
        return CABINET_OK;
    }
    if (this->addChildren(newChild) == CABINET_ERR) {
        logWarning("add child into children error");
        delete newChild;
        return CABINET_OK;
    }
    logDebug("cluster cluster_id[%d] connect lost children success", clusterId);
    return CABINET_OK;
}

ClusterClient *Children::getOnlineChildren() const {
    if (this->connectStatus == true) {
        return this->child;
    }
    return nullptr;
}

int Children::shutDown() {
    int connectFd = this->child->getClientFd();
    Util::closeConnectFd(connectFd);
    return CABINET_OK;
}
