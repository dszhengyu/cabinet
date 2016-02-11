#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <map>
#include "ValueObj.h"

using std::string;
using std::map;

class DataBase
{
public:
    typedef map<string, ValueObj *> dataspace_t;
    DataBase();
    dataspace_t getDataSpace() {return this->dataSpace;}
    int deleteKey(const string &key);
    ValueObj *getValue(const string &key);

private:
    dataspace_t dataSpace;
};
#endif
