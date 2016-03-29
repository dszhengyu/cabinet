#ifndef CLIENT_H
#define CLIENT_H


#include <string>
#include "Cabinet.h"
#include "CommandKeeper.h"
#include "EventPoll.h"
#include "ProtocolStream.h"
#include "PersistenceFile.h"

class Cabinet;
class CommandKeeper;
class EventPoll;
class ProtocolStream;
class PersistenceFile;

using std::string;

class Client
{
public:
    Client(long clientId, int fd, const string &ip, const int port, Cabinet *cabinet);
    long getClientId() const {return this->clientId;}
    const string &getIp() const {return this->ip;}
    int getClientFd() const {return this->fd;}

    //protocolstream proxy method
    int fillReceiveBuf();
    int fillReceiveBuf(const string &str);
    int fillSendBuf(const string &str);
    int resolveReceiveBuf();
    const vector<string> &getReceiveArgv() const {return this->protocolStream.getReceiveArgv();}
    bool isReceiveComplete() const {return this->protocolStream.isReceiveComplete();}
    const string &getCurCommandBuf() const {return this->protocolStream.getCurCommandBuf();}
    int sendReply();
    int initReplyHead(int argc);
    int appendReplyType(const char commandType);
    int appendReplyBody(const string &);
    int appendReplyBody(const char *);

    //category relative
    char getCategory() const {return this->category;}
    void setCategory(const char newCategory) {this->category = newCategory;}
    static const char NORMAL_CLIENT = '\0';
    static const char LOCAL_PF_CLIENT = 'L';
    static const char CLUSTER_CLIENT = 'C';
    static const char SERVER_CLIENT = 'S';

    int resetClient();
    void getReadyToSendMessage();
    virtual int executeCommand() = 0;
    virtual ~Client();

protected:
    long clientId;
    CommandKeeper *commandKeeper;
    int fd;
    string ip;
    int port;
    EventPoll *eventPoll;
    ProtocolStream protocolStream;
    PersistenceFile *pf;
    char category;
    const int READ_MAX_LEN = 1024 * 16;

};
#endif
