#ifndef SIBLINGS_H
#define SIBLINGS_H

#include "Cluster.h"
#include "Configuration.h"
class Cluster;
class Configuration;

class Siblings
{
public:
    Siblings();
    int recognizeSiblings(const Configuration &conf);
private:
};
#endif
