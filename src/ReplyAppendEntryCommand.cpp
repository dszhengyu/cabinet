#include "ReplyAppendEntryCommand.h"
#include "Cluster.h"
#include "ClusterClient.h"
#include <algorithm>

/*
 *brief: role: all
 *  argc: term
 *        success
 *possible reply:
 *      1. higher term, to follow
 *      2. false, decrease nextIndex
 *      3. success, set matchIndex = nextIndex, increase nextIndex
 */
int ReplyAppendEntryCommand::operator[](Client *client) const {
    ClusterClient *clusterClient = (ClusterClient *) client;
    Cluster *cluster = clusterClient->getClusterPtr();
    int leaderId = cluster->getClusterId();
    int followerId = clusterClient->getClusterId();
    logNotice("cluster cluster_id[%d] receive reply append entry from cluster[%d]", leaderId, followerId);

    const vector<string> &argv = clusterClient->getReceiveArgv();
    if (argv.size() != (unsigned int)(this->commandArgc() + 1)) {
        logFatal("request command receive error number of argv! argv_len[%d]", argv.size());
        exit(1);
    }   

    if (!cluster->isLeader()) {
        logNotice("cluster cluster_id[%d] receive reply append entry from cluster[%d], not a leader now, ignore",
                leaderId, followerId);
        return CABINET_OK;
    }

    long followerTerm;
    const string &result = argv[4];
    try {
        followerTerm = std::stol(argv[2]);
    } catch (std::exception &e) {
        logFatal("receive argc error, receive exception, what[%s]", e.what());
        exit(1);
    }

    long term = cluster->getTerm();
    if (term < followerTerm) {
        logNotice("cluster cluster_id[%d] receive reply append entry with high term, to follow", leaderId);
        cluster->toFollow(followerTerm);
    }

    Siblings *siblings = cluster->getSiblings();
    if (result == string("false")) {
        long currentNextIndex = siblings->getSiblingNextIndex(followerId);
        if (currentNextIndex == 1) {
            logWarning("can not decrease next index to 0, program fail");
            return CABINET_OK;
        }
        siblings->decreaseSiblingNextIndex(followerId);
        return CABINET_OK;
    }
    else {
        long newMatchIndex = siblings->getSiblingNextIndex(followerId);
        siblings->increaseSiblingNextIndex(followerId);
        siblings->setMatchIndex(followerId, newMatchIndex);
        return CABINET_OK;
    }
}
