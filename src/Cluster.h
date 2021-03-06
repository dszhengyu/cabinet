#ifndef CLUSTER_H
#define CLUSTER_H

#include "Cabinet.h"
#include "Siblings.h"
#include "Children.h"
#include "Parents.h"
#include "Client.h"
#include "ClusterClient.h"
class Cabinet;
class Siblings;
class Children;
class Parents;
class Client;
class ClusterClient;

class Cluster: public Cabinet
{
public:
    Cluster();
    void initConfig();
    void init();
    Client *createClient(int listenFd);
    ClusterClient *createNormalClient(int connectFd, const string &ip, const int port);
    int deleteClient(Client *client);
    int cron();
    int nextCronTime();
    Siblings *getSiblings() const {return this->siblings;}

    int toLead();
    int toFollow(long newTerm);
    int toCandidate();
    bool isLeader() const {return this->getClusterRole() == Cluster::LEADER;}
    bool isFollower() const {return this->getClusterRole() == Cluster::FOLLOWER;}
    bool isCandidate() const {return this->getClusterRole() == Cluster::CANDIDATE;}
    long updateTimeout();

    int getClusterId() const {return this->clusterId;}
    long getTerm() const {return this->currentTerm;}
    long getIndex() const {return this->commitIndex;}
    void setIndex(long index) {this->commitIndex = index;}
    long getLastApplied() const {return this->lastApplied;}
    void increaseLastApplied() {++this->lastApplied;}
    int alreadyVotedFor() const {return this->votedFor;}
    void voteFor(int candidateId) {this->votedFor = candidateId;}
    void increaseVote() {++this->receiveVotes;}
    bool achieveLeaderBaseline() const {return this->receiveVotes >= this->winVoteBaseline;}
    int getVoteCount() const {return this->receiveVotes;}

    int setNewEntryIndexAndTerm(Entry &newEntry);
    Parents *getParents() const {return this->parents;}

    int shutDown();
    ~Cluster();
private:
    const char getClusterRole() const {return this->role;}
    void setClusterRole(const char newRole) {this->role = newRole;}
    int clusterId;
    char role;
    long currentTerm;
    int votedFor;
    long commitIndex;
    long lastEntryIndex;
    long lastApplied;
    Siblings *siblings;
    Children *children;
    Parents *parents;
    bool meetWorkingBaseline;
    long lastUnixTimeInMs;
    int hz;
    int electionTimeout;
    int receiveVotes;
    int winVoteBaseline;
    string pfName;

    static const char LEADER = 'L';
    static const char FOLLOWER = 'F';
    static const char CANDIDATE = 'C';
};
#endif
