#include "Cabinet.h"
#include "Util.h"
#include "Log.h"
#include "Const.h"
#include <unistd.h>

Cabinet::Cabinet() :
    clientIdMax(0),
    port(-1),
    listenFd(-1),
    commandKeeperPtr(nullptr),
    eventPoll(nullptr),
    pf(nullptr)
{

} 

int Cabinet::listenOnPort(int port) {
    int listenFd;
    if ((listenFd = Util::listenTcp(port)) == CABINET_ERR) {
        logFatal("listen on port error, port[%d]", port);
        return CABINET_ERR;
    }
    return listenFd;
}

/* 
 * brief: 获取客户端连接
 *      成功返回描述符, 失败打印日志, 返回错误
 */
int Cabinet::getConnectFd(int listenFd, string &strIP, int &port) {
    logDebug("server get client connect");
    int connectFd;
    if ((connectFd = Util::acceptTcp(listenFd, strIP, port)) == CABINET_ERR) {
        logWarning("accept connect error");
        return CABINET_ERR;
    }

    logNotice("receive client connect, client_ip[%s], client_port[%d]", strIP.c_str(), port);
    return connectFd;
}

void Cabinet::onFire() const {
    logDebug("cabinet on fire");
    this->eventPoll->processEvent();
}

Cabinet::~Cabinet() {
    if (this->listenFd != -1) {
        close(this->listenFd);
    }
    delete this->eventPoll;
    delete this->commandKeeperPtr;
}
