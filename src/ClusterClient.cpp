#include "ClusterClient.h"
#include "Const.h"

ClusterClient::ClusterClient(long clientId, int fd, const string &ip, const int port, Cluster *cluster) :
    Client(clientId, fd, ip, port, cluster),
    cluster(cluster),
    clusterId(-1)
{
}

int ClusterClient::executeCommand() {
    //select command
    const string &commandName = this->protocolStream.getCommandName();
    Command &selectedCommand = this->commandKeeper->selectCommand(commandName);

    //execute
    if (selectedCommand[this] == CABINET_ERR) {
        logWarning("cluster client client_id[%d] execute command error", this->getClientId());
        return CABINET_ERR;
    }
    this->protocolStream.getReadyForNextCommand();
    return CABINET_OK;
}

ClusterClient::~ClusterClient() {

}

