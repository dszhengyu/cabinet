#ifndef SERVER_H
#define SERVER_H

/*****************************************************************************/
#define SERVER_ID 1
#define CABINET_PORT 8080
const bool PF = true;
/*****************************************************************************/

#include "Cabinet.h"
#include "DataBase.h"
#include "Const.h"
class Cabinet;
class DataBase;

class Server: public Cabinet
{
public :
    Server();
    void initConfig();
    void init();
    Client *createClient(const int connectFd, const string &ip, const int port);
    int deleteClient(Client *client) {return CABINET_OK;}
    int cron() {return CABINET_OK;}
    int nextCronTime() {return -1;}
    int importPF();
    ~Server();

private:
    long serverId;
    DataBase *db;
};

#endif
