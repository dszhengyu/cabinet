#include "StringObj.h"

StringObj::StringObj():
    StringObj(string())
{
}

StringObj::StringObj(const string &initValue):
    stringValue(initValue)
{
    this->valueType = ValueObj::STRING_TYPE;
}

