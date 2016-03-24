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
    vector<ClusterClient *> getOnlineSiblings();
    bool satisfyWorkingBaseling();
    int connectLostSiblings();

    long getSiblingNextIndex(int clusterId);
    int inscreaseSiblingNextIndex(int clusterId);
    int inscreaseSiblingMatchIndex(int clusterId);

private:
    int validateClusterId(int clusterId);
    int clusterId;
    vector<int> clusterIdVector;
    map<int, string> ipMap;
    map<int, int> portMap;
    map<int, long> nextIndexMap;
    map<int, long> matchIndexMap;
    map<int, bool> connectStatus;
    map<int, ClusterClient*> clusterIdClientPtrMap;
    Cluster *cluster;
};
#endif
