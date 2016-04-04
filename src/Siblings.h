#ifndef SIBLINGS_H
#define SIBLINGS_H

#include "Cluster.h"
#include "ClusterClient.h"
#include "Configuration.h"
#include <vector>
#include <string>
class Cluster;
class ClusterClient;
class Configuration;
using std::vector;
using std::string;

class Siblings
{
public:
    Siblings(Cluster *cluster);
    int recognizeSiblings(Configuration &conf);
    int addSiblings(ClusterClient *sibling);
    int deleteSiblings(ClusterClient *sibling);
    vector<ClusterClient *> getSiblingsNeedAppendEntry();
    bool satisfyWorkingBaseling();
    int connectLostSiblings();

    long getSiblingNextIndex(int clusterId);
    int increaseSiblingNextIndex(int clusterId);
    int decreaseSiblingNextIndex(int clusterId);
    void setNextIndexBatch(long newNextIndex);
    void setMatchIndexBatch(long newMatchIndex);
    void setMatchIndex(int clusterId, long newMatchIndex);
    int setAlreadyAppendEntry(int clusterId, bool already);
    void setAlreadyAppendEntryBatch(bool alreadyBatch);
    int setEmptyAppendEntry(int clusterId, bool empty);
    int getEmptyAppendEntry(const int clusterId, bool &empty);
    void checkIfEntryCouldCommit(long index);

    int getLeaderId() const {return this->currentLeaderId;}
    void setLeaderId(int leaderId) {this->currentLeaderId = leaderId;}
    int clusterTotalNumber() const {return this->clusterIdVector.size() + 1;}
    int clusterHalfNumber() const {return this->clusterTotalNumber() / 2 + 1;}

    int getLeaderIPAndPort(string &ip, int &port);
    int confirmConnectSibling(ClusterClient *sibling);
    int shutDown();

private:
    int validateClusterId(int clusterId);
    int getSiblingClusterId(ClusterClient *sibling);
    int clusterId;
    vector<int> clusterIdVector;
    map<int, string> ipMap;
    map<int, int> portMap;
    map<int, long> nextIndexMap;
    map<int, long> matchIndexMap;
    map<int, bool> connectStatus;
    map<int, ClusterClient*> clusterIdClientPtrMap;
    Cluster *cluster;
    int currentLeaderId;
    map<string, int> ipPortClusterIdMap;
    map<int, bool> connectTrying;
    map<int, bool> alreadyAppendEntry;
    map<int, bool> emptyAppendEntry;
};
#endif
