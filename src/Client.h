#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include "CommandKeeper.h"
#include "EventPoll.h"
#include "ProtocolStream.h"

class CommandKeeper;
class EventPoll;
class ProtocolStream;

using std::string;

class Client
{
public:
    Client(long clientId, CommandKeeper *commandKeeper, int fd, EventPoll *eventPoll);
    long getClientId() const {return this->clientId;}
    int fillInputBuf();
    int resolveInputBuf();
    int executeCommand();
    int sendReply();
    int initReplyHead(int argc);
    int appendReplyBody(const string &);
    int appendReplyBody(const char *);
    int getClientFd() const {return this->fd;}
    bool isReadyToExecute() const {return this->protocolStream.isReadyToExecute();}

private:
    long clientId;
    string outputBuf;
    CommandKeeper *commandKeeper;
    int fd;
    EventPoll *eventPoll;
    ProtocolStream protocolStream;
    const int READ_MAX_LEN = 1024 * 16;
};
#endif
