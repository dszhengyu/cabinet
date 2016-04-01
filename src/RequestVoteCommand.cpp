#include "RequestVoteCommand.h"
#include "Cluster.h"
#include "ClusterClient.h"

/*
 * brief: role: Candidate
 *  argc: term
 *        candidateId
 *        lastLogIndex
 *        lastLogTerm
 */
int RequestVoteCommand::operator>>(Client *client) const {
    ClusterClient *clusterClient = (ClusterClient *) client;
    Cluster *cluster = clusterClient->getClusterPtr();
    if (!cluster->isCandidate()) {
        logFatal("cluster is not a candidate, but request vote!");
        exit(1);
    }

    int ticketHolderId = clusterClient->getClusterId();
    long term = cluster->getTerm();
    int candidateId = cluster->getClusterId();
    PersistenceFile *pf = cluster->getPersistenceFile();
    Entry lastEntry;
    pf->findLastEntry(lastEntry);
    long lastLogIndex = lastEntry.getIndex();
    long lastLogTerm = lastEntry.getTerm();

    logDebug("cluster cluster_id[%d] send request vote to cluster[%d]", candidateId, ticketHolderId);
    client->initReplyHead(9);
    client->appendReplyType(this->commandType());
    client->appendReplyBody("requestvote");
    client->appendReplyBody("term");
    client->appendReplyBody(std::to_string(term));
    client->appendReplyBody("candidateId");
    client->appendReplyBody(std::to_string(candidateId));
    client->appendReplyBody("lastLogIndex");
    client->appendReplyBody(std::to_string(lastLogIndex));
    client->appendReplyBody("lastLogTerm");
    client->appendReplyBody(std::to_string(lastLogTerm));
    return CABINET_OK;
}

/*
 *brief: role: all
 *  argc: term
 *        candidateId
 *        lastLogIndex
 *        lastLogTerm
 *  response argc: term
 *                 voteGranted
 */
int RequestVoteCommand::operator[](Client *client) const {
    ClusterClient *clusterClient = (ClusterClient *) client;
    Cluster *cluster = clusterClient->getClusterPtr();
    int ticketHolderId = cluster->getClusterId();

    const vector<string> &argv = clusterClient->getReceiveArgv();
    if (argv.size() != (unsigned int)(this->commandArgc() + 1)) {
        logFatal("request command receive error number of argv! argv_len[%d]", argv.size());
        exit(1);
    }   

    long candidateTerm;
    int candidateId;
    long candidateLastLogIndex;
    long candidateLastLogTerm;
    try {
        candidateTerm = std::stol(argv[2]);
        candidateId = std::stoi(argv[4]);
        candidateLastLogIndex = std::stol(argv[6]);
        candidateLastLogTerm = std::stol(argv[8]);
    } catch (std::exception &e) {
        logFatal("receive request vote argc error, receive exception, what[%s]", e.what());
        exit(1);
    }
    logDebug("cluster cluster_id[%d] receive request vote from cluster[%d]", ticketHolderId, candidateId);

    long term = cluster->getTerm();
    PersistenceFile *pf = cluster->getPersistenceFile();
    Entry lastEntry;
    pf->findLastEntry(lastEntry);
    long lastLogIndex = lastEntry.getIndex();
    long lastLogTerm = lastEntry.getTerm();

    if (term < candidateTerm) {
        logNotice("cluster cluster_id[%d] receive request vote from cluster[%d] has high term, to follow", 
                ticketHolderId, candidateId);
        cluster->toFollow(candidateTerm);
        term = cluster->getTerm();
    }
    if ((term > candidateTerm) || 
            (lastLogTerm > candidateLastLogTerm) || 
            (lastLogIndex > candidateLastLogIndex) ||
            (cluster->alreadyVotedFor() != -1)) {
        client->initReplyHead(5);
        client->appendReplyType(this->commandType());
        client->appendReplyBody("replyrequestvote");
        client->appendReplyBody("term");
        client->appendReplyBody(std::to_string(term));
        client->appendReplyBody("voteGranted");
        client->appendReplyBody("false");
        return CABINET_OK;
    }

    cluster->voteFor(candidateId);
    client->initReplyHead(5);
    client->appendReplyType(this->commandType());
    client->appendReplyBody("replyrequestvote");
    client->appendReplyBody("term");
    client->appendReplyBody(std::to_string(term));
    client->appendReplyBody("voteGranted");
    client->appendReplyBody("true");
    return CABINET_OK;
}
