#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include "CommandKeeper.h"
#include "EventPoll.h"
#include "DataBase.h"
#include "ProtocolStream.h"

class CommandKeeper;
class EventPoll;
class DataBase;
class ProtocolStream;

using std::string;

class Client
{
public:
    Client(long clientId, CommandKeeper *commandKeeper, int fd, EventPoll *eventPoll, DataBase *dataBasePtr);
    long getClientId() const {return this->clientId;}
    int fillReceiveBuf();
    int resolveReceiveBuf();
    const vector<string> &getReceiveArgv() const {return this->protocolStream.getReceiveArgv();}
    int executeCommand();
    int sendReply();
    int initReplyHead(int argc);
    int appendReplyBody(const string &);
    int appendReplyBody(const char *);
    int getClientFd() const {return this->fd;}
    bool isReceiveComplete() const {return this->protocolStream.isReceiveComplete();}
    DataBase *getDataBase() const {return this->dataBasePtr;}
    ~Client();

private:
    long clientId;
    CommandKeeper *commandKeeper;
    int fd;
    EventPoll *eventPoll;
    DataBase *dataBasePtr;
    ProtocolStream protocolStream;
    const int READ_MAX_LEN = 1024 * 16;
};
#endif
