#ifndef CHILDREN_H
#define CHILDREN_H

#include "Cluster.h"
#include "ClusterClient.h"
#include "Configuration.h"
#include <vector>
#include <map>
class Cluster;
class ClusterClient;
class Configuration;
using std::vector;
using std::map;

class Children
{
public:
    Children(Cluster *cluster);
    int recognizeChildren(Configuration &conf);
    int deleteChildren(ClusterClient *child);
    bool satisfyWorkingBaseling();
    int connectLostChildren();
    int flushServer();
    int shutDown();
private:
    int addChildren(int id, ClusterClient *child);
    int getChildrenId(ClusterClient *child);
    Cluster *cluster;
    vector<int> serverIdVector;
    map<int, ClusterClient *>childPtrMap;
    map<int, bool> connectStatus;
    map<int, string> ipMap;
    map<int, int> portMap;
};
#endif

