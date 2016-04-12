#ifndef SERVER_H
#define SERVER_H

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
    Client *createClient(int listenFd);
    int deleteClient(Client *client) {return CABINET_OK;}
    int cron() {return CABINET_OK;}
    int nextCronTime() {return -1;}
    int importPF();
    ~Server();

private:
    DataBase *db;
    string pfName;
    string allowPF;
};

#endif
