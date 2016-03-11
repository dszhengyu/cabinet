#ifndef SIBLINGS_H
#define SIBLINGS_H

#include "ClusterClient.h"
#include "Configuration.h"
#include <vector>
#include <string>
class ClusterClient;
class Configuration;
using std::vector;
using std::string;

class Siblings
{
public:
    Siblings();
    int recognizeSiblings(Configuration &conf);
    int addSiblings(int clusterId, ClusterClient *sibling);
    int deleteSiblings(ClusterClient *sibling);
    vector<ClusterClient *> getOnlineSiblings();
    bool satisfyWorkingBaseling();
    int connectLostSiblings();

private:
    int clusterId;
    vector<int> clusterIdVector;
    map<int, string> ipMap;
    map<int, int> portMap;
    map<int, long> nextIndexMap;
    map<int, long> matchIndexMap;
    map<int, bool> connectStatus;
    map<int, ClusterClient*> clusterIdClientPtrMap;
};
#endif
