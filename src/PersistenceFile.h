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
    PersistenceFile(const string &pfFileName, const string &tmpPFName);

    int findEntry(const long index, Entry &entry);
    int findLastEntry(Entry &entry);
    int deleteEntryAfter(long index);

    int appendToPF(const Entry &entry);
    int getNextPFEntry(Entry &entry);

    ~PersistenceFile();
private:
    int resetFileStream();
    string pFName;
    string tmpPFName;
    ofstream pFOut;
    ifstream pFIn;
};
#endif
