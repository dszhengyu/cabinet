#ifndef CLUSTERCLIENT_H
#define CLUSTERCLIENT_H

#include "Cluster.h"
class Cluster;

class ClusterClient: public Client
{
public:
    int executeCommand();
    ~ClusterClient();

private:
    Cluster *cluster;
    long dealingEntryIndex;
};
#endif
