#ifndef SERVER_H
#define SERVER_H

/*****************************************************************************/
#define SERVER_ID 1
#define CABINET_PORT 8080
/*****************************************************************************/

#include "CommandKeeper.h"
#include "Client.h"
#include "EventPoll.h"
class CommandKeeper;
class Client;
class EventPoll;

class Server
{
public :
    Server();
    void initConfig();
    void init();
    Client *createClient(int connectFd);
    int listenOnPort();
    int getListenFd() const {return this->listenFd;}
    int getConnectFd();
    ~Server();

private:
    long serverId;
    long clientIdMax;
    int port;
    int listenFd;
    CommandKeeper *commandKeeperPtr;
    EventPoll *eventPoll;
    //DataBase *db;
};

#endif
