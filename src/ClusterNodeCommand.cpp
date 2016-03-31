#include "ClusterNodeCommand.h"
#include "Cluster.h"
#include "ClusterClient.h"
#include <algorithm>

/*
 * brief: role: all
 *  argc: clusterId
 */
int ClusterNodeCommand::operator>>(Client *client) const {
    ClusterClient *clusterClient = (ClusterClient *) client;
    Cluster *cluster = clusterClient->getClusterPtr();
    int clusterId = cluster->getClusterId();

    logDebug("cluster cluster_id[%d] send cluster node message to cluster[%d]",
            clusterId, clusterClient->getClusterId());

    client->initReplyHead(3);
    client->appendReplyType(this->commandType());
    client->appendReplyBody("clusternode");
    client->appendReplyBody("clusterId");
    client->appendReplyBody(std::to_string(clusterId));
    return CABINET_OK;
}

/*
 *brief: role: all
 *  argc: clusterId
 *  response argc: clusterId
 *                 success
 */
int ClusterNodeCommand::operator[](Client *client) const {
    ClusterClient *clusterClient = (ClusterClient *) client;
    Cluster *cluster = clusterClient->getClusterPtr();

    const vector<string> &argv = clusterClient->getReceiveArgv();
    if (argv.size() != (unsigned int)(this->commandArgc() + 1)) {
        logFatal("cluster node command receive error number of argv! argv_len[%d]", argv.size());
        exit(1);
    }   

    int clusterId = cluster->getClusterId();
    int siblingId;
    try {
        siblingId = std::stoi(argv[2]);
    } catch (std::exception &e) {
        logFatal("receive request vote argc error, receive exception, what[%s]", e.what());
        exit(1);
    }

    logDebug("cluster cluster_id[%d] receive cluster node message from cluster[%d]",
            clusterId, siblingId);

    Parents *parents = cluster->getParents();
    if (parents->deleteParents(clusterClient) == CABINET_ERR) {
        logWarning("cluster cluster_id[%d] trying to delete parents and add to sibling error",
                clusterId);
        return CABINET_ERR;
    }
    clusterClient->setClusterId(siblingId);
    clusterClient->setCategory(Client::CLUSTER_CLIENT);
    Siblings *siblings = cluster->getSiblings();
    if (siblings->addSiblings(clusterClient) == CABINET_ERR) {
        logWarning("cluster cluster_id[%d] trying to add to sibling error", clusterId);
        return CABINET_ERR;
    }
    siblings->confirmConnectSibling(clusterClient);

    client->initReplyHead(5);
    client->appendReplyType(this->commandType());
    client->appendReplyBody("replyclusternode");
    client->appendReplyBody("clusterId");
    client->appendReplyBody(std::to_string(clusterId));
    client->appendReplyBody("success");
    client->appendReplyBody("true");
    return CABINET_OK;
}
