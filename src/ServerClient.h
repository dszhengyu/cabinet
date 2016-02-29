#ifndef SERVERCLIENT_H
#define SERVERCLIENT_H

#include "Client.h"
#include "DataBase.h"
class Client;
class DataBase;

class ServerClient: public Client
{
public:
    ServerClient(long clientId, CommandKeeper *commandKeeper, int fd, const string &ip, const int port, 
            EventPoll *eventPoll, PersistenceFile *pf, DataBase *dataBasePtr);
    int executeCommand();
    DataBase *getDataBase() const {return this->dataBasePtr;}

protected:
    DataBase *dataBasePtr;
};

#endif
