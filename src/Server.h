#ifndef SERVER_H
#define SERVER_H

/*****************************************************************************/
#define SERVER_ID 1
#define CABINET_PORT 8080
/*****************************************************************************/

#include "CommandKeeper.h"
#include <memory>
using std::shared_ptr;
using std::make_shared;

class Server
{
public :
    Server();
    void initConfig();
    void init();
    //Client *createClient(int connectFd);
    int getListenFd();
    int getConnectFd(int listenFd);

private:
    long serverId;
    int port;
    int listenFd;
    //shared_ptr<CommandKeeper> commandKeeperPtr;
    CommandKeeper *commandKeeperPtr;
    //EventPoll *eventPoll;
    //DataBase *db;
};

#endif
