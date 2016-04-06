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
    int leaderId = cluster->getClusterId();
    logDebug("cluster cluster_id[%d] append entry to cluster[%d]", leaderId, clientId);
    Siblings *siblings = cluster->getSiblings();
    long nextEntryIndex = siblings->getSiblingNextIndex(clientId);
    if (nextEntryIndex == CABINET_ERR || nextEntryIndex < 1) {
        logFatal("cluster cluster_id[%d] append entry into invalid sibling with wrong cluster_id!");
        exit(1);
    }

    long leaderTerm = cluster->getTerm();
    long commitIndex = cluster->getIndex();
    PersistenceFile *pf = cluster->getPersistenceFile();
    Entry lastEntry;
    pf->findLastEntry(lastEntry);
    long lastLogIndex = lastEntry.getIndex();

    Entry nextEntry;
    long index = 0;
    long term = 0;
    string entryCommand;
    //next log not exist yet
    if (pf->findEntry(nextEntryIndex, nextEntry) == CABINET_ERR) {
        if (nextEntryIndex > (lastLogIndex + 1)) {
            logNotice("cluster cluster_id[%d] not contain entry to append, next_index[%ld] change to follower", 
                    leaderId, nextEntryIndex);
            cluster->toFollow(cluster->getTerm());
            return CABINET_OK;
        }
        //set empty append entry
        if (siblings->setEmptyAppendEntry(clientId, true) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] set empty append entry cluster[%d] to [true] error");
            return CABINET_ERR;
        }
    }
    else {
        index = nextEntry.getIndex();
        term = nextEntry.getTerm();
        entryCommand = nextEntry.getContent();
        //set empty append entry
        if (siblings->setEmptyAppendEntry(clientId, false) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] set empty append entry cluster[%d] to [false] error");
            return CABINET_ERR;
        }
    }

    Entry entryBeforeNextEntry;
    long prevLogIndex = 0;
    long prevLogTerm = 0;
    if (nextEntryIndex > 1) {
        if (pf->findEntry(nextEntryIndex - 1, entryBeforeNextEntry) == CABINET_ERR) {
            logWarning("cluster cluster_id[%d] lose entry before next_index[%ld] when append entry to cluster[%d]",
                leaderId, clientId);
            cluster->toFollow(cluster->getTerm());
            return CABINET_OK;
        }
        prevLogIndex = entryBeforeNextEntry.getIndex();
        prevLogTerm = entryBeforeNextEntry.getTerm();
    }

    if (siblings->setAlreadyAppendEntry(clientId, true) == CABINET_ERR) {
        logWarning("cluster cluster_id[%d] set cluster[%d] already append entry[true] error", leaderId, clientId);
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
    client->printSendBuf();
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
    int followerId = cluster->getClusterId();

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

    logDebug("cluster cluster_id[%d] receive append entry from cluster[%d]", followerId, leaderId);


    //验证term的正确性
    //1. 如果当前主机是follower, 收到的term大于它, 变为新的follower
    //2. 如果当前主机是candidate, 收到的term大于等于它就变成follower
    //3. 如果当前主机是leader, 收到的term大于它, 变为follower, 等于它, fatal一下, 变为follower
    long term = cluster->getTerm();
    if (cluster->isFollower()) {
        if (term < leaderTerm) {
            logNotice("cluster cluster_id[%d] follower receive append entry from cluster[%d] with higher term, to follow", 
                    followerId, leaderId);
            cluster->toFollow(leaderTerm);
            term = cluster->getTerm();
        }
    }
    else if (cluster->isCandidate()) {
        if (term <= leaderTerm) {
            logNotice("cluster cluster_id[%d] candidate receive append entry from cluster[%d] with euqal or high term, to follow", 
                    followerId, leaderId);
            cluster->toFollow(leaderTerm);
            term = cluster->getTerm();
        }
        else {
            logDebug("cluster cluster_id[%d] is candidate with higher term, reject append entry from cluster[%d]", followerId, leaderId);
            client->initReplyHead(5);
            client->appendReplyType(this->commandType());
            client->appendReplyBody("replyappendentry");
            client->appendReplyBody("term");
            client->appendReplyBody(std::to_string(term));
            client->appendReplyBody("success");
            client->appendReplyBody("false");
            client->printSendBuf();
            return CABINET_OK;
        }
    }
    else if (cluster->isLeader()) {
        if (term < leaderTerm) {
            logNotice("cluster cluster_id[%d] leader receive append entry from cluster[%d] with higher term, to follow", 
                    followerId, leaderId);
            cluster->toFollow(leaderTerm);
            term = cluster->getTerm();
        }
        else if (term == leaderTerm) {
            logNotice("cluster cluster_id[%d] leader receive append entry from cluster[%d] with equal term, to follow", 
                    followerId, leaderId);
            cluster->toFollow(leaderTerm);
            term = cluster->getTerm();
        }
        else {
            logDebug("cluster cluster_id[%d] is leader with higher term, reject append entry from cluster[%d]", 
                    followerId, leaderId);
            client->initReplyHead(5);
            client->appendReplyType(this->commandType());
            client->appendReplyBody("replyappendentry");
            client->appendReplyBody("term");
            client->appendReplyBody(std::to_string(term));
            client->appendReplyBody("success");
            client->appendReplyBody("false");
            client->printSendBuf();
            return CABINET_OK;
        }
    }

    if (!cluster->isFollower()) {
        logFatal("only follower execute here");
        exit(1);
    }

    //重要: 运行到此, 表示验证成功(追随这个leader)之后, 一切log都要跟此leader一致
    //更改某些状态
    cluster->updateTimeout();
    Siblings *siblings = cluster->getSiblings();
    siblings->setLeaderId(leaderId);
    PersistenceFile *pf = cluster->getPersistenceFile();

    //验证prevLog的正确性
    //  两种情况: 1. prevLogIndex为0, 只需验证本机pf也没有entry即可. 如果有, 全部删除
    //            2. prevLogIndex不为0, 详细验证prevLogIndex以及prevLogTerm
    if (leaderPrevLogIndex == 0) {
        Entry lastEntry;
        if (pf->findLastEntry(lastEntry) != CABINET_ERR) {
            logDebug("cluster cluster_id[%d] get prevLogIndex[0] from cluster[%d], but have log in local pf,\
                    reject append entry from cluster[%d], delete all the log in local of",
                    followerId, leaderId, leaderId);
            pf->deleteAllEntry();
            client->initReplyHead(5);
            client->appendReplyType(this->commandType());
            client->appendReplyBody("replyappendentry");
            client->appendReplyBody("term");
            client->appendReplyBody(std::to_string(term));
            client->appendReplyBody("success");
            client->appendReplyBody("false");
            client->printSendBuf();
            return CABINET_OK;
        }
    }
    else {
        Entry prevEntry;
        //确认leaderPrevLogIndex 对应的entry在本机的log之中
        if (pf->findEntry(leaderPrevLogIndex, prevEntry) == CABINET_ERR) {
            logDebug("cluster cluster_id[%d] can not find leader prevlog in pf, reject append entry from cluster[%d]", 
                followerId, leaderId);
            client->initReplyHead(5);
            client->appendReplyType(this->commandType());
            client->appendReplyBody("replyappendentry");
            client->appendReplyBody("term");
            client->appendReplyBody(std::to_string(term));
            client->appendReplyBody("success");
            client->appendReplyBody("false");
            client->printSendBuf();
            return CABINET_OK;
        }

        //确认对应的entry的term也是正确的
        if (prevEntry.getTerm() != leaderPrevLogTerm) {
            logDebug("cluster cluster_id[%d] find leader prev log in pf with same index but different term, \
                    reject append entry from cluster[%d]", 
                    followerId, leaderId);
            if (pf->deleteEntryAfter(leaderPrevLogIndex) == CABINET_ERR) {
                logFatal("cluster cluster_id[%d] get cluster[%d] append entry, deleting entry error", followerId, leaderId);
            }
            client->initReplyHead(5);
            client->appendReplyType(this->commandType());
            client->appendReplyBody("replyappendentry");
            client->appendReplyBody("term");
            client->appendReplyBody(std::to_string(term));
            client->appendReplyBody("success");
            client->appendReplyBody("false");
            client->printSendBuf();
            return CABINET_OK;
        }

        //如果这个entry不是最后一个entry, 删除它之后的, 使它成为最后一个entry
        Entry lastEntry;
        if (pf->findLastEntry(lastEntry) == CABINET_ERR) {
            logFatal("cluster cluster_id[%d] should not execute here, program fail, please check", followerId);
            exit(1);
        }
        if ((lastEntry.getIndex() != leaderPrevLogIndex) || (lastEntry.getTerm() != leaderPrevLogTerm)) {
            logDebug("cluster cluster_id[%d] get cluster[%d] append entry, find prev entry, but not the last entry,\
                    deleting the entry after the prev(no including the prev entry)",
                    followerId, leaderId);
            if (pf->deleteEntryAfter(leaderPrevLogIndex + 1) == CABINET_ERR) {
                logFatal("cluster cluster_id[%d] get cluster[%d] append entry, deleting entry error", followerId, leaderId);
                client->initReplyHead(5);
                client->appendReplyType(this->commandType());
                client->appendReplyBody("replyappendentry");
                client->appendReplyBody("term");
                client->appendReplyBody(std::to_string(term));
                client->appendReplyBody("success");
                client->appendReplyBody("false");
                client->printSendBuf();
                return CABINET_OK;
            }
        }
    }

    //检查是否有新的entry需要append
    if (leaderEntryIndex != 0 && leaderEntryTerm != 0) {
        //append entry
        Entry newEntry(leaderEntryIndex, leaderEntryTerm, entryCommand);
        if (pf->appendToPF(newEntry) == CABINET_ERR) {
            logFatal("cluster cluster_id[%d] get cluster[%d] append entry, appending entry error", followerId, leaderId);
            client->initReplyHead(5);
            client->appendReplyType(this->commandType());
            client->appendReplyBody("replyappendentry");
            client->appendReplyBody("term");
            client->appendReplyBody(std::to_string(term));
            client->appendReplyBody("success");
            client->appendReplyBody("false");
            client->printSendBuf();
            return CABINET_OK;
        }
    }

    //检查是否有entry在leader已经commit, 如果有, 更新当前主机的commitIndex
    if (leaderCommit != 0) {
        Entry lastEntry;
        if (pf->findLastEntry(lastEntry) == CABINET_ERR) {
            logFatal("cluster cluster_id[%d] should execute correct here, persistence file fail, please check", followerId);
            exit(1);
        }
        long newCommitIndex = std::min(leaderCommit, lastEntry.getIndex());
        cluster->setIndex(newCommitIndex);
    }

    client->initReplyHead(5);
    client->appendReplyType(this->commandType());
    client->appendReplyBody("replyappendentry");
    client->appendReplyBody("term");
    client->appendReplyBody(std::to_string(term));
    client->appendReplyBody("success");
    client->appendReplyBody("true");
    client->printSendBuf();
    return CABINET_OK;
}
