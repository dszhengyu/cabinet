#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <vector>
#include "CommandKeeper.h"

using std::string;
using std::vector;

class Client
{
public:
    Client(long clientId, CommandKeeper *commandKeeperPtr);
    int fillInputBuff(int connectFd);
    int resolveInputBuff();
    int executeCommand();
    int sendReply();
    long getClientId() const {return this->clientId;}

private:
    long clientId;
    string inputBuf;
    string outputBuf;
    int argc;//argc三种状态: -1, 没有命令;0, 命令解析完成, 等待执行; >0, 命令还在解析中
    vector<string> argv;
    int curArgvLen;
    int curArgc;
    CommandKeeper *commandKeeperPtr;
    //EventPoll *eventPoll;
};
#endif
