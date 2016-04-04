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

    long lastApplied = cluster->getLastApplied();
    long commitIndex = cluster->getIndex();

    if (commitIndex == lastApplied) {
        return CABINET_OK; 
    }

    if (commitIndex < lastApplied) {
        logFatal("something wrong with commitIndex and lastApplied, program fail");
        exit(1);
    }

    cluster->increaseLastApplied();
    long sendingEntryIndex = cluster->getLastApplied();
    PersistenceFile *pf = cluster->getPersistenceFile();
    Entry sendingEntry;
    if (pf->findEntry(sendingEntryIndex, sendingEntry) == CABINET_ERR) {
        logWarning("cluster cluster_id[%d] could not find the entry to flush to server",
                cluster->getClusterId());
        return CABINET_ERR;
    }
    const string &content = sendingEntry.getContent();
    client->fillSendBuf(content);
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

    const vector<string> &argv = clusterClient->getReceiveArgv();
    if (argv.size() != (unsigned int)(this->commandArgc() + 1)) {
        logFatal("flush server get wrong number of argv! argv_len[%d]", argv.size());
        exit(1);
    }   

    const string &serverReply = argv[1];
    long flushedEntryIndex = cluster->getLastApplied();
    Parents *parents = cluster->getParents();
    ClusterClient *parent = parents->getParentsByDealingIndex(flushedEntryIndex);
    if (parent == nullptr) {
        logNotice("cabinet client exit before get reply");
        return CABINET_OK;
    }
    parent->fillSendBuf(serverReply);
    parent->getReadyToSendMessage();
    return CABINET_OK;
}
