#include "AppendEntryCommand.h"
#include "Cluster.h"
#include "ClusterClient.h"

/*
 * brief: role: Leader
 *  argc: term
 *        leaderId
 *        prevLogIndex
 *        prevLogTerm
 *        index
 *        term
 *        entryCommand
 *        leaderCommit
 *notice: 1. get sibling next index
 *        2. get entry, and the entry before
 *        3. get leader commit index
 */
int AppendEntryCommand::operator>>(Client *client) const {
    ClusterClient *clusterClient = (ClusterClient *) client;
    Cluster *cluster = clusterClient->getClusterPtr();
    if (!cluster->isLeader()) {
        logFatal("cluster is not a leader, but append entry!");
        exit(1);
    }

    int clientId = clusterClient->getClusterId();
    Siblings *siblings = cluster->getSiblings();
    long nextEntryIndex = siblings->getSiblingNextIndex(clientId);
    if (nextEntryIndex == CABINET_ERR) {
        logFatal("append entry into invalid sibling with wrong cluster id!");
        exit(1);
    }

    long leaderTerm = cluster->getTerm();
    long commitIndex = cluster->getIndex();
    int leaderId = cluster->getClusterId();
    PersistenceFile *pf = cluster->getPersistenceFile();

    long index = 0;
    long term = 0;
    string entryCommand;
    if (nextEntryIndex > 0) {
        Entry nextEntry;
        pf->findEntry(nextEntryIndex, nextEntry);
        index = nextEntry.getIndex();
        term = nextEntry.getTerm();
        entryCommand = nextEntry.getContent();
    }

    long prevLogIndex = 0;
    long prevLogTerm = 0;
    if (nextEntryIndex > 1) {
        Entry entryBeforeNextEntry;
        pf->findEntry(nextEntryIndex - 1, entryBeforeNextEntry);
        prevLogIndex = entryBeforeNextEntry.getIndex();
        prevLogTerm = entryBeforeNextEntry.getTerm();
    }

    client->initReplyHead(17);
    client->appendReplyType(this->commandType());
    client->appendReplyBody("appendentry");
    client->appendReplyBody("term");
    client->appendReplyBody(std::to_string(leaderTerm));
    client->appendReplyBody("leaderId");
    client->appendReplyBody(std::to_string(leaderId));
    client->appendReplyBody("prevLogIndex");
    client->appendReplyBody(std::to_string(prevLogIndex));
    client->appendReplyBody("prevLogTerm");
    client->appendReplyBody(std::to_string(prevLogTerm));
    client->appendReplyBody("index");
    client->appendReplyBody(std::to_string(index));
    client->appendReplyBody("term");
    client->appendReplyBody(std::to_string(term));
    client->appendReplyBody("entryCommand");
    client->appendReplyBody(entryCommand);
    client->appendReplyBody("leaderCommit");
    client->appendReplyBody(std::to_string(commitIndex));
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
int AppendEntryCommand::operator[](Client *client) const {
    ClusterClient *clusterClient = (ClusterClient *) client;
    Cluster *cluster = clusterClient->getClusterPtr();

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

    long term = cluster->getTerm();
    PersistenceFile *pf = cluster->getPersistenceFile();
    Entry lastEntry;
    pf->findLastEntry(lastEntry);
    long lastLogIndex = lastEntry.getIndex();
    long lastLogTerm = lastEntry.getTerm();

    if (term < candidateTerm) {
        logNotice("cluster cluster_id[%d] receive request vote has high term, to follow", 
                cluster->getClusterId());
        cluster->toFollow(candidateTerm);
        term = cluster->getTerm();
    }
    if ((term > candidateTerm) || 
            (lastLogTerm > candidateLastLogTerm) || 
            (lastLogIndex > candidateLastLogIndex) ||
            (cluster->alreadyVotedFor() != -1)) {
        client->initReplyHead(5);
        client->appendReplyType(this->commandType());
        client->appendReplyBody("replyRequestVote");
        client->appendReplyBody("term");
        client->appendReplyBody(std::to_string(term));
        client->appendReplyBody("voteGranted");
        client->appendReplyBody("false");
        return CABINET_OK;
    }

    cluster->voteFor(candidateId);
    client->initReplyHead(5);
    client->appendReplyType(this->commandType());
    client->appendReplyBody("replyRequestVote");
    client->appendReplyBody("term");
    client->appendReplyBody(std::to_string(term));
    client->appendReplyBody("voteGranted");
    client->appendReplyBody("true");
    return CABINET_OK;
}
