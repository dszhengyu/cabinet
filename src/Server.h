#ifndef SERVER_H
#define SERVER_H

/*****************************************************************************/
#define SERVER_ID 1
#define CABINET_PORT 8080
/*****************************************************************************/

#include "CommandKeeper.h"
#include "Client.h"

class Server
{
public :
    Server();
    void initConfig();
    void init();
    Client *createClient(int connectFd);
    void listenOnPort();
    int getConnectFd();
    ~Server();

private:
    long serverId;
    long clientIdMax;
    int port;
    int listenFd;
    CommandKeeper *commandKeeperPtr;
    //EventPoll *eventPoll;
    //DataBase *db;
};

#endif
