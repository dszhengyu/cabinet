#ifndef CHILDREN_H
#define CHILDREN_H

#include "ClusterClient.h"
#include "Configuration.h"
class ClusterClient;
class Configuration;

class Children
{
public:
    Children();
    int recognizeChildren(const Configuration &conf);
    int addChildren(ClusterClient *child);
    int deleteChildren(ClusterClient *child);
    bool satisfyWorkingBaseling() const;
    int connectLostChildren();
    ClusterClient *getOnlineChildren() const;
private:
    ClusterClient *child;
    bool connectStatus;
};
#endif

