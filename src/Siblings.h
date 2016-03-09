#ifndef SIBLINGS_H
#define SIBLINGS_H

#include "ClusterClient.h"
#include "Configuration.h"
#include <vector>
class ClusterClient;
class Configuration;
using std::vector;

class Siblings
{
public:
    Siblings();
    int recognizeSiblings(const Configuration &conf);
    int addSiblings(ClusterClient *sibling);
    int deleteSiblings(ClusterClient *sibling);
    vector<ClusterClient *> getOnlineSiblings();

private:
    vector<int> clusterIdVector;
    map<int, string> ipMap;
    map<int, int> portMap;
    map<int, long> nextIndexMap;
    map<int, long> matchIndexMap;
    map<int, bool> connectStatus;
    map<int, ClusterClient*> clusterIdClientPtrMap;
};
#endif
