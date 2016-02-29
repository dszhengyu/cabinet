#ifndef CABINET_H
#define CABINET_H

#include "CommandKeeper.h"
#include "Client.h"
#include "EventPoll.h"
#include <string>
class CommandKeeper;
class Client;
class EventPoll;

using std::string;

class Cabinet
{
public:
    Cabinet();
    virtual void initConfig() = 0;
    virtual void init() = 0;
    virtual Client *createClient(const int connectFd, const string &ip, const int port) = 0;
    void onFire() const;
    int listenOnPort();
    int getListenFd() const {return this->listenFd;}
    int getConnectFd(string &ip, int &port);
    virtual ~Cabinet();

protected:
    long clientIdMax;
    int port;
    int listenFd;
    CommandKeeper *commandKeeperPtr;
    EventPoll *eventPoll;

};
#endif
