#include "Client.h"
#include "Const.h"

Client::Client(long clientId, CommandKeeper *commandKeeperPtr):
    clientId(clientId),
    inputBuf(),
    outputBuf(),
    argc(-1),
    argv(),
    curArgvLen(-1),
    curArgc(-1),
    commandKeeperPtr(commandKeeperPtr)
{

}

/* 
 * brief: 当客户端对应套接字可读时调用
 *      将接收到的内容放入输入缓冲区
 */
int Client::fillInputBuff(int connectFd) {

    return CABINET_OK;
}

/* 
 * brief: 当输入缓冲区中有可解析的内容时调用
 *      解析客户端输入缓冲区的内容
 *      失败返回 错误, 需要清除客户端及其连接
 */
int Client::resolveInputBuff() {

    return CABINET_OK;
}

/* 
 * brief: 当输入缓冲区中当前命令解析完毕时调用
 */
int Client::executeCommand() {

    return CABINET_OK;
}
