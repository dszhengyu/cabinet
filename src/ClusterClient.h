#ifndef CLUSTERCLIENT_H
#define CLUSTERCLIENT_H

#include "Cluster.h"
class Cluster;

class ClusterClient: public Client
{
public:
    ClusterClient(long clientId, int fd, const string &ip, const int port, Cluster *cluster);
    int executeCommand();
    Cluster *getClusterPtr() const {return this->cluster;}
    int getClusterId() const {return this->clusterId;}
    ~ClusterClient();

private:
    Cluster *cluster;
    int clusterId;
};
#endif
