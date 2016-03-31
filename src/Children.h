#ifndef CHILDREN_H
#define CHILDREN_H

#include "Cluster.h"
#include "ClusterClient.h"
#include "Configuration.h"
class Cluster;
class ClusterClient;
class Configuration;

class Children
{
public:
    Children(Cluster *cluster);
    int recognizeChildren(Configuration &conf);
    int addChildren(ClusterClient *child);
    int deleteChildren(ClusterClient *child);
    bool satisfyWorkingBaseling() const;
    int connectLostChildren();
    ClusterClient *getOnlineChildren() const;
    int shutDown();
private:
    Cluster *cluster;
    ClusterClient *child;
    bool connectStatus;
    string ip;
    int port;
};
#endif

