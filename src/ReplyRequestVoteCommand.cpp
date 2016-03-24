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
    if (!cluster->isCandidate()) {
        //reply might come after the role has changed, just ignore it 
        logWarning("cluster is not a candidate, ignore reply request vote!");
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
        logNotice("cluster cluster_id[%d] receive reply request vote has high term, to follow", 
                cluster->getClusterId());
        cluster->toFollow(replyTerm);
        return CABINET_OK;
    }

    if (voteGranted != string("true")) {
        logNotice("cluster cluster_id[%d] receive reply request vote, but not get a vote", 
                cluster->getClusterId());
        return CABINET_OK;
    }

    cluster->increaseVote();
    logNotice("cluster cluster_id[%d] receive reply request vote, get a vote", 
            cluster->getClusterId());
    if (cluster->achieveLeaderBaseline()) {
        logNotice("cluster cluster_id[%d] receive enough vote, change to leader",
                cluster->getClusterId());
        cluster->toLead();
        return CABINET_OK;
    }

    return CABINET_OK;
}
