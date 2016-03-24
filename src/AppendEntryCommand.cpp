#include "AppendEntryCommand.h"
#include "Cluster.h"
#include "ClusterClient.h"
#include <algorithm>

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
    Entry lastEntry;
    pf->findLastEntry(lastEntry);
    long lastLogIndex = lastEntry.getIndex();

    Entry nextEntry;
    if (nextEntryIndex > 0) {
        if ((pf->findEntry(nextEntryIndex, nextEntry) == CABINET_ERR) &&
                (nextEntryIndex > (lastLogIndex + 1))) {
            logNotice("cluster cluster_id[%d] does not contain necessary entry to append, change to follower", 
                    cluster->getClusterId());
            cluster->toFollow(cluster->getTerm());
            return CABINET_OK;
        }
    }
    long index = nextEntry.getIndex();
    long term = nextEntry.getTerm();
    string entryCommand = nextEntry.getContent();

    Entry entryBeforeNextEntry;
    if (nextEntryIndex > 1) {
        pf->findEntry(nextEntryIndex - 1, entryBeforeNextEntry);
    }
    long prevLogIndex = entryBeforeNextEntry.getIndex();
    long prevLogTerm = entryBeforeNextEntry.getTerm();

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
 *        leaderId
 *        prevLogIndex
 *        prevLogTerm
 *        index
 *        term
 *        entryCommand
 *        leaderCommit
 *  response argc: term
 *                 success
 *notice:
 *      1. if leader or candidate receive this and term is higher than itself
 *          change to follower
 *      2. index and term both are 0 mean this is a heartbeat
 *      3. prevLogIndex and prevLogTerm should be check as long as the command is not stale
 */
int AppendEntryCommand::operator[](Client *client) const {
    ClusterClient *clusterClient = (ClusterClient *) client;
    Cluster *cluster = clusterClient->getClusterPtr();

    const vector<string> &argv = clusterClient->getReceiveArgv();
    if (argv.size() != (unsigned int)(this->commandArgc() + 1)) {
        logFatal("request command receive error number of argv! argv_len[%d]", argv.size());
        exit(1);
    }   

    long leaderTerm;
    int leaderId;
    long leaderPrevLogIndex;
    long leaderPrevLogTerm;
    long leaderEntryIndex;
    long leaderEntryTerm;
    const string &entryCommand = argv[14];
    long leaderCommit;
    try {
        leaderTerm = std::stol(argv[2]);
        leaderId = std::stoi(argv[4]);
        leaderPrevLogIndex = std::stol(argv[6]);
        leaderPrevLogTerm = std::stol(argv[8]);
        leaderEntryIndex = std::stol(argv[10]);
        leaderEntryTerm = std::stol(argv[12]);
        leaderCommit = std::stol(argv[16]);
    } catch (std::exception &e) {
        logFatal("receive request vote argc error, receive exception, what[%s]", e.what());
        exit(1);
    }

    long term = cluster->getTerm();
    if (cluster->isCandidate() || cluster->isLeader()) {
        if (term < leaderTerm) {
            logNotice("cluster cluster_id[%d] receive append entry with high term, to follow", 
                    cluster->getClusterId());
            cluster->toFollow(leaderTerm);
            term = cluster->getTerm();
        }
        else {
            client->initReplyHead(5);
            client->appendReplyType(this->commandType());
            client->appendReplyBody("replyappendentry");
            client->appendReplyBody("term");
            client->appendReplyBody(std::to_string(term));
            client->appendReplyBody("success");
            client->appendReplyBody("false");
            return CABINET_OK;
        }
    }

    if (!cluster->isFollower()) {
        logFatal("only follower execute here");
        exit(1);
    }

    cluster->updateTimeout();
    Siblings *siblings = cluster->getSiblings();
    siblings->setLeaderId(leaderId);
    PersistenceFile *pf = cluster->getPersistenceFile();
    Entry prevEntry;
    if (pf->findEntry(leaderPrevLogIndex, prevEntry) == CABINET_ERR) {
        client->initReplyHead(5);
        client->appendReplyType(this->commandType());
        client->appendReplyBody("replyappendentry");
        client->appendReplyBody("term");
        client->appendReplyBody(std::to_string(term));
        client->appendReplyBody("success");
        client->appendReplyBody("false");
        return CABINET_OK;
    }

    if (prevEntry.getTerm() != leaderPrevLogTerm) {
        pf->deleteEntryAfter(leaderPrevLogIndex);
        client->initReplyHead(5);
        client->appendReplyType(this->commandType());
        client->appendReplyBody("replyappendentry");
        client->appendReplyBody("term");
        client->appendReplyBody(std::to_string(term));
        client->appendReplyBody("success");
        client->appendReplyBody("false");
        return CABINET_OK;
    }

    if (leaderEntryIndex != 0 && leaderEntryTerm != 0) {
        //append entry
        Entry newEntry(leaderEntryIndex, leaderEntryTerm, entryCommand);
        pf->appendToPF(newEntry);
        long newCommitIndex = std::min(leaderCommit, cluster->getIndex());
        cluster->setIndex(newCommitIndex);
    }
    client->initReplyHead(5);
    client->appendReplyType(this->commandType());
    client->appendReplyBody("replyappendentry");
    client->appendReplyBody("term");
    client->appendReplyBody(std::to_string(term));
    client->appendReplyBody("success");
    return CABINET_OK;
}
