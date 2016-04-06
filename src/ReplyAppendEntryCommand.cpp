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
    //logDebug("cluster cluster_id[%d] receive reply append entry from cluster[%d]", leaderId, followerId);

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

    //验证term
    long term = cluster->getTerm();
    if (term < followerTerm) {
        logNotice("cluster cluster_id[%d] receive reply append entry from cluster[%d] with higher term, to follow", 
                leaderId, followerId);
        cluster->toFollow(followerTerm);
        return CABINET_OK;
    }
    if (!cluster->isLeader()) {
        logFatal("only leader execute here, program fail");
        exit(1);
    }

    //允许继续append entry
    Siblings *siblings = cluster->getSiblings();
    if (siblings->setAlreadyAppendEntry(followerId, false) == CABINET_ERR) {
        logWarning("cluster cluster_id[%d] set cluster[%d] already append entry[true] error", leaderId, followerId);
    }

    //验证append entry结果
    if (result == string("false")) {
        //logDebug("cluster cluster_id[%d] receive cluster[%d] append entry reply[false], decrease next_index", leaderId, followerId);
        long currentNextIndex = siblings->getSiblingNextIndex(followerId);
        if (currentNextIndex > 1) {
            siblings->decreaseSiblingNextIndex(followerId);
        }
        return CABINET_OK;
    }
    if (result == string("true")) {
        //logDebug("cluster cluster_id[%d] receive cluster[%d] append entry reply[true]", leaderId, followerId);
        //检验之前append entry 是否是空的
        bool empty;
        if (siblings->getEmptyAppendEntry(followerId, empty) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] get cluster[%d] empty append entry error", leaderId, followerId);
            return CABINET_OK;
        }
        if (empty == true) {
            //logDebug("cluster cluster_id[%d] last append entry cluster[%d] empty[true]", leaderId, followerId);
            return CABINET_OK;
        }
        //append 不为空的entry且获得了成功
        //logDebug("cluster cluster_id[%d] last append entry cluster[%d] empty[false]", leaderId, followerId);
        long newMatchIndex = siblings->getSiblingNextIndex(followerId);
        siblings->increaseSiblingNextIndex(followerId);
        siblings->setMatchIndex(followerId, newMatchIndex);
        siblings->checkIfEntryCouldCommit(newMatchIndex);
        return CABINET_OK;
    }

    logWarning("cluster cluster_id[%d] receive cluster[%d] append entry with unidentified result[%s]", 
            leaderId, followerId, result.c_str());
    return CABINET_ERR;
}
