#include "ReplyClusterNodeCommand.h"
#include "Cluster.h"
#include "ClusterClient.h"
#include <algorithm>

/*
 *brief: role: all
 *  argc: clusterId
 *  response argc: clusterId
 *                 success
 */
int ReplyClusterNodeCommand::operator[](Client *client) const {
    ClusterClient *clusterClient = (ClusterClient *) client;
    Cluster *cluster = clusterClient->getClusterPtr();

    const vector<string> &argv = clusterClient->getReceiveArgv();
    if (argv.size() != (unsigned int)(this->commandArgc() + 1)) {
        logFatal("reply cluster node command receive error number of argv! argv_len[%d]", argv.size());
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
    logDebug("cluster cluster_id[%d] receive reply cluster node message from cluster[%d]",
            clusterId, siblingId);

    Siblings *siblings = cluster->getSiblings();
    if (siblings->confirmConnectSibling(clusterClient) == CABINET_ERR) {
        logWarning("cluster cluster_id[%d] trying to add to sibling error", clusterId);
        return CABINET_ERR;
    }

    return CABINET_OK;
}
