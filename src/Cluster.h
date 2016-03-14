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

    int toLead();
    int toFollow(long newTerm);
    int toCandidate();

    int getClusterId() const {return this->clusterId;}
    const char getClusterRole() const {return this->role;}
    void setClusterRole(const char newRole) {this->role = newRole;}
    ~Cluster();
private:
    int clusterId;
    char role;
    long currentTerm;
    int votedFor;
    long commitIndex;
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
    int clusterPort;
    int clusterListenFd;

    static const char LEADER = 'L';
    static const char FOLLOWER = 'F';
    static const char CANDIDATE = 'C';
};
#endif
