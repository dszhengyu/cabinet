#ifndef PERSISTENCEFILE_H
#define PERSISTENCEFILE_H

#include "Client.h"
#include "Entry.h"
#include <string>
#include <fstream>

using std::ofstream;
using std::ifstream;

class Client;
class Entry;

using std::string;

class PersistenceFile 
{
public:
    PersistenceFile();
    int appendToPF(const Entry &entry);
    int initReadPF();
    int endReadPF();
    int readNextPFEntry();
    const Entry &getCurPFEntry() {return this->curPFEntry;}
    ~PersistenceFile();
private:
    Entry curPFEntry;
    string pFName;
    ifstream pFIn;
    ofstream pFOut;
    constexpr const static char *serverPFName = "cabinet-server.pf";
    constexpr const static char *clusterPFName = "cabinet-cluster.pf";
};
#endif
