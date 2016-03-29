#include "ServerClient.h"
#include "Entry.h"

ServerClient::ServerClient(long clientId, int fd, const string &ip, const int port, DataBase *dataBasePtr, Cabinet *cabinet) :
    Client(clientId, fd, ip, port, cabinet),
    dataBasePtr(dataBasePtr)
{

}

/* 
 * brief: 当输入缓冲区中当前命令解析完毕时调用
 * pf: when it is not a pf client and pf is require, pf need pf command
 *
 */
int ServerClient::executeCommand() {
    logNotice("server client execute command");
    const string &commandName = this->protocolStream.getCommandName();
    Command &selectedCommand = this->commandKeeper->selectCommand(commandName);

    logDebug("client category[%c]", this->category);

    //do pf
    if ((this->category != Client::LOCAL_PF_CLIENT) && 
            (pf != nullptr) &&
            (selectedCommand.needPF() == true)) {
        Entry entry(this->protocolStream.getCurCommandBuf());
        this->pf->appendToPF(entry);
    }

    //execute
    if (selectedCommand(this) == CABINET_ERR) {
        logWarning("server client client_id[%d] execute command error", this->getClientId());
        return CABINET_ERR;
    }
    return CABINET_OK;
}

ServerClient::~ServerClient()
{

}
