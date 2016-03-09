#include "ClusterClient.h"
#include "Const.h"

ClusterClient::ClusterClient(long clientId, int fd, const string &ip, const int port, Cluster *cluster) :
    Client(clientId, fd, ip, port, cluster),
    cluster(cluster)
{
}

int ClusterClient::executeCommand() {
    return CABINET_OK;
}

ClusterClient::~ClusterClient() {

}

