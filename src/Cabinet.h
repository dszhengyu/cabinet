#ifndef CABINET_H
#define CABINET_H

#include "CommandKeeper.h"
#include "Client.h"
#include "EventPoll.h"
#include "PersistenceFile.h"
#include <string>
class CommandKeeper;
class Client;
class EventPoll;
class PersistenceFile;

using std::string;

class Cabinet
{
public:
    Cabinet();
    virtual void initConfig() = 0;
    virtual void init() = 0;
    virtual Client *createClient(int listenFd) = 0;
    virtual int deleteClient(Client *client) = 0;
    virtual int cron() = 0;
    virtual int nextCronTime() = 0;
    void onFire() const;
    CommandKeeper *getCommandKeeper() const {return this->commandKeeperPtr;}
    EventPoll *getEventPoll() const {return this->eventPoll;}
    PersistenceFile *getPersistenceFile() const {return this->pf;}
    virtual ~Cabinet();

protected:
    int listenOnPort(int port);
    int getConnectFd(int listenFd, string &ip, int &port);
    int getListenFd() const {return this->listenFd;}
    long clientIdMax;
    int port;
    int listenFd;
    CommandKeeper *commandKeeperPtr;
    EventPoll *eventPoll;
    PersistenceFile *pf;
};
#endif
