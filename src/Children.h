#ifndef CHILDREN_H
#define CHILDREN_H

#include "Cluster.h"
#include "Configuration.h"
class Cluster;
class Configuration;

class Children
{
public:
    Children();
    int recognizeChildren(const Configuration &conf);
private:
    Cluster *cluster;
};
#endif

