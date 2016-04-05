#include "FlushServerCommand.h"
#include "Cluster.h"
#include "ClusterClient.h"

/*
 * brief: role: all
 * argc: the content in entry
 */
int FlushServerCommand::operator>>(Client *client) const {
    ClusterClient *clusterClient = (ClusterClient *) client;
    Cluster *cluster = clusterClient->getClusterPtr();
    int clusterId = cluster->getClusterId();
    //logDebug("cluster cluster_id[%d] check if need to flush command to server", clusterId);


    long lastApplied = cluster->getLastApplied();
    long commitIndex = cluster->getIndex();

    if (commitIndex == lastApplied) {
        //logDebug("cluster cluster_id[%d] not need to flush command to server", clusterId);
        return CABINET_OK; 
    }

    if (commitIndex < lastApplied) {
        logFatal("something wrong with commitIndex and lastApplied, program fail");
        exit(1);
    }

    cluster->increaseLastApplied();
    long sendingEntryIndex = cluster->getLastApplied();
    logDebug("cluster cluster_id[%d] flush command to server, index[%ld]", clusterId, sendingEntryIndex);
    PersistenceFile *pf = cluster->getPersistenceFile();
    Entry sendingEntry;
    if (pf->findEntry(sendingEntryIndex, sendingEntry) == CABINET_ERR) {
        logWarning("cluster cluster_id[%d] could not find the entry to flush to server", clusterId);
        return CABINET_ERR;
    }
    const string &content = sendingEntry.getContent();
    client->fillSendBuf(content);
    client->printSendBuf();
    client->getReadyToSendMessage();

    return CABINET_OK;
}

/*
 *brief: role: all
 *argc: reply
 *notice:
 *  1. write the response to the client
 */
int FlushServerCommand::operator[](Client *client) const {
    ClusterClient *clusterClient = (ClusterClient *) client;
    Cluster *cluster = clusterClient->getClusterPtr();
    int clusterId = cluster->getClusterId();
    logDebug("cluster cluster_id[%d] receive reply for flush server", clusterId);

    const vector<string> &argv = clusterClient->getReceiveArgv();
    if (argv.size() != (unsigned int)(this->commandArgc() + 1)) {
        logFatal("flush server get wrong number of argv! argv_len[%d]", argv.size());
        exit(1);
    }   

    if (!cluster->isLeader()) {
        logDebug("cluster cluster_id[%d] is not leader, not need to flush server", clusterId);
        return CABINET_OK;
    }

    const string &serverReply = argv[1];
    long flushedEntryIndex = cluster->getLastApplied();
    Parents *parents = cluster->getParents();
    ClusterClient *parent = parents->getParentsByDealingIndex(flushedEntryIndex);
    if (parent == nullptr) {
        logNotice("cluster cluster_id[%d] cabinet client exit before get reply", clusterId);
        return CABINET_OK;
    }
    logDebug("cluster cluster_id[%d] reply client", clusterId);
    parent->fillSendBuf(serverReply);
    parent->printSendBuf();
    parent->getReadyToSendMessage();
    return CABINET_OK;
}
