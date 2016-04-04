#include "ClusterDefaultCommand.h"
#include "Cluster.h"
#include "ClusterClient.h"

/*
 *brief: for all role, receive client request
 *  1. leader:
 *      a. add to entry
 *      b. set dealing index and other parents attribute
 *  2. follower:
 *      redirect to leader ip and port
 *  3. candidate:
 *      retry
 */
int ClusterDefaultCommand::operator[](Client *client) const {
    ClusterClient *clusterClient = (ClusterClient *) client;
    Cluster *cluster = clusterClient->getClusterPtr();
    int clusterId = cluster->getClusterId();
    logDebug("cluster cluster[%d] receive client request", clusterId);

    if (cluster->isLeader()) {
        const string &wholeCommand = client->getCurCommandBuf();
        Entry newEntry(wholeCommand);
        cluster->setNewEntryIndexAndTerm(newEntry);
        long newEntryIndex = newEntry.getIndex();
        PersistenceFile *pf = cluster->getPersistenceFile();
        if (pf->appendToPF(newEntry) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] append to pf fail, change to follower", clusterId);
            cluster->toFollow(cluster->getTerm());
            return CABINET_OK;
        }
        Parents *parents = cluster->getParents();
        if (parents->setDealingIndex(newEntryIndex, clusterClient) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] set parents dealing index fail", clusterId);
            return CABINET_ERR;
        }
        return CABINET_OK;
    }

    if (cluster->isFollower()) {
        Siblings *siblings = cluster->getSiblings();
        string leaderIP;
        int leaderPort;
        if (siblings->getLeaderIPAndPort(leaderIP, leaderPort) == CABINET_ERR) {
            logWarning("follower get leader ip and port error");
            return CABINET_ERR;
        }
        client->initReplyHead(5);
        client->appendReplyBody("redirect");
        client->appendReplyBody("leaderip");
        client->appendReplyBody(leaderIP);
        client->appendReplyBody("leaderport");
        client->appendReplyBody(std::to_string(leaderPort));
        return CABINET_OK;
    }

    if (cluster->isCandidate()) {
        client->initReplyHead(1);
        client->appendReplyBody("retry");
        return CABINET_OK;
    }

    return CABINET_OK;
}
