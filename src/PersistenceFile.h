#ifndef PERSISTENCEFILE_H
#define PERSISTENCEFILE_H

#include "Client.h"
#include <string>
#include <fstream>

using std::ofstream;
using std::ifstream;

class Client;

using std::string;

class PersistenceFile 
{
public:
    PersistenceFile();
    int appendToPF(Client *);
    int initReadPF();
    int endReadPF();
    int readNextPFEntry();
    const string &getCurPFEntry() {return this->curPFEntry;}
    ~PersistenceFile();
private:
    string curPFEntry;
    string pFName;
    ifstream pFIn;
    ofstream pFOut;
    const static char delimiter = '\r';
    constexpr const static char *serverPFName = "cabinet-server.pf";
};
#endif
