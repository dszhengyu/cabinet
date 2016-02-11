#ifndef VALUEOBJ_H
#define VALUEOBJ_H

#include <string>

using std::string;

class ValueObj 
{
public:
    ValueObj(): valueType(0) {};
    ~ValueObj(){};
    const int getValueType() const {return this->valueType;}

    static const int STRING_TYPE = 1;

protected:
    int valueType;
};
#endif
