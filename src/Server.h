#ifndef SERVER_H
#define SERVER_H

const bool PF = true;

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
    Client *createClient();
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
