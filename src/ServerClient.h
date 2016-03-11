#ifndef SERVERCLIENT_H
#define SERVERCLIENT_H

#include "Client.h"
#include "Cabinet.h"
#include "DataBase.h"
class Client;
class Cabinet;
class DataBase;

class ServerClient: public Client
{
public:
    ServerClient(long clientId, int fd, const string &ip, const int port, DataBase *dataBasePtr, Cabinet *cabinet);
    int executeCommand();
    DataBase *getDataBase() const {return this->dataBasePtr;}

protected:
    DataBase *dataBasePtr;
};

#endif
