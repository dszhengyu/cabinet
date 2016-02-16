#ifndef SERVER_H
#define SERVER_H

/*****************************************************************************/
#define SERVER_ID 1
#define CABINET_PORT 8080
const bool PF = true;
/*****************************************************************************/

#include "CommandKeeper.h"
#include "Client.h"
#include "EventPoll.h"
#include "DataBase.h"
#include "PersistenceFile.h"
class CommandKeeper;
class Client;
class EventPoll;
class DataBase;
class PersistenceFile;

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
    void onFire() const;
    int importPF();
    ~Server();

private:
    long serverId;
    long clientIdMax;
    int port;
    int listenFd;
    CommandKeeper *commandKeeperPtr;
    EventPoll *eventPoll;
    DataBase *db;
    PersistenceFile *pf;
};

#endif
