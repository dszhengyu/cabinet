#ifndef CLUSTER_H
#define CLUSTER_H

#include "Cabinet.h"
#include "Siblings.h"
#include "Children.h"
#include "Parents.h"
#include "Client.h"
class Cabinet;
class Siblings;
class Children;
class Parents;
class Client;

class Cluster: public Cabinet
{
public:
    Cluster();
    void initConfig();
    void init();
    Client *createClient(const int connectFd, const string &ip, const int port);
    int deleteClient(Client *client);
    int cron();
    int nextCronTime();

    int toLead();
    int toFollow();
    int toCandidate();

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

    static const char LEADER = 'L';
    static const char FOLLOWER = 'F';
    static const char CANDIDATE = 'C';
};
#endif
