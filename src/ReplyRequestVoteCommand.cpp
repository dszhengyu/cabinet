#include "ReplyRequestVoteCommand.h"
#include "Cluster.h"
#include "ClusterClient.h"

/*
 * brief: role: Candidate
 *  argc: term
 *        voteGranted
 */
int ReplyRequestVoteCommand::operator[](Client *client) const {
    ClusterClient *clusterClient = (ClusterClient *) client;
    Cluster *cluster = clusterClient->getClusterPtr();
    int clusterId = cluster->getClusterId();
    int voterId = clusterClient->getClusterId();
    //logDebug("cluster cluster_id[%d] receive reply for request vote from cluster[%d]", clusterId, voterId);
    if (!cluster->isCandidate()) {
        //reply might come after the role has changed, just ignore it 
        logWarning("cluster cluster_id[%d] is not a candidate, ignore reply request vote from cluster[%d]", clusterId, voterId);
        return CABINET_OK;
    }


    long term = cluster->getTerm();
    long replyTerm;
    const vector<string> &argv = clusterClient->getReceiveArgv();
    const string &voteGranted = argv[4];

    try {
        replyTerm = std::stol(argv[2]);
    } catch (std::exception &e) {
        logFatal("receive reply request vote argc error, receive exception, what[%s]", e.what());
        exit(1);
    }
    
    if (replyTerm > term) {
        logNotice("cluster cluster_id[%d] receive reply request vote from cluster[%d] has higher term, to follow", 
                clusterId, voterId);
        cluster->toFollow(replyTerm);
        return CABINET_OK;
    }

    if (voteGranted != string("true")) {
        logNotice("cluster cluster_id[%d] receive reply request vote from cluster[%d], but not get a vote", clusterId, voterId);
        return CABINET_OK;
    }

    cluster->increaseVote();
    logNotice("cluster cluster_id[%d] receive reply request vote from cluster[%d], get a vote, total[%d]", clusterId, voterId,
            cluster->getVoteCount());
    if (cluster->achieveLeaderBaseline()) {
        logNotice("cluster cluster_id[%d] receive enough vote[%d], change to leader", clusterId, cluster->getVoteCount());
        cluster->toLead();
        return CABINET_OK;
    }

    return CABINET_OK;
}
