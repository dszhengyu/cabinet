#ifndef SERVER_H
#define SERVER_H

/*****************************************************************************/
#define SERVER_ID 1
#define CABINET_PORT 8080
/*****************************************************************************/

#include <string>
#include <map>
#include "Command.h"

using std::string;
using std::map;

class Server
{
public :
    typedef map<string, Command *> commandmap_t;
    Server();
    void initConfig();
    void init();
    void createCommandMap();
    //Client *createClient(int connectFd);
    int getListenFd();
    int getConnectFd(int listenFd);

private:
    long serverId;
    int port;
    int listenFd;
    commandmap_t *commandMap;
    //EventPoll *eventPoll;
    //DataBase *db;
};

#endif
