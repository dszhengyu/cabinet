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
    Children();
    int recognizeChildren(const Configuration &conf);
    int addChildren(ClusterClient *child);
    int deleteChildren(ClusterClient *child);
private:
    Cluster *cluster;
};
#endif

