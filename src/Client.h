#ifndef CLIENT_H
#define CLIENT_H


#include <string>
#include "CommandKeeper.h"
#include "EventPoll.h"
#include "DataBase.h"
#include "ProtocolStream.h"
#include "PersistenceFile.h"

class CommandKeeper;
class EventPoll;
class DataBase;
class ProtocolStream;
class PersistenceFile;

using std::string;

class Client
{
public:
    Client(long clientId, CommandKeeper *commandKeeper, int fd, EventPoll *eventPoll, DataBase *dataBasePtr, PersistenceFile *pf);
    long getClientId() const {return this->clientId;}
    int fillReceiveBuf();
    int fillReceiveBuf(const string &str);
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
    char getCategory() const {return this->category;}
    void setCategory(const char newCategory) {this->category = newCategory;}
    const string &getCurCommandBuf() const {return this->protocolStream.getCurCommandBuf();}
    int resetClient();
    ~Client();
    static const char NORMAL_CLIENT = '\0';
    static const char LOCAL_PF_CLIENT = 'L';

private:
    long clientId;
    CommandKeeper *commandKeeper;
    int fd;
    EventPoll *eventPoll;
    DataBase *dataBasePtr;
    ProtocolStream protocolStream;
    const int READ_MAX_LEN = 1024 * 16;
    char category;
    PersistenceFile *pf;
};
#endif
