#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <vector>
#include "CommandKeeper.h"
class CommandKeeper;

using std::string;
using std::vector;

class Client
{
public:
    Client(long clientId, CommandKeeper *commandKeeperPtr, int fd);
    int fillInputBuf();
    int resolveInputBuf();
    int executeCommand();
    int sendReply();
    long getClientId() const {return this->clientId;}
    int initReplyHead(int argc);
    int appendReplyBody(const string &);
    int appendReplyBody(const char *);

private:
    int isInputBufAvaliable() const;
    int resetBufForNextCommand();
    long clientId;
    string inputBuf;
    string outputBuf;
    int argc;//argc三种状态: -1, 没有命令;0, 命令解析完成, 等待执行; >0, 命令还在解析中
    vector<string> argv;
    long curArgvLen;
    CommandKeeper *commandKeeperPtr;
    int fd;
    const int READ_MAX_LEN = 1024 * 16;
    bool readyToExecute;
    char commandType;
    //EventPoll *eventPoll;
};
#endif
