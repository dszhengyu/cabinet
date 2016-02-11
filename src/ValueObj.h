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

    //string object
    static const int STRING_TYPE = 1;
    virtual const string &get() const = 0;
    virtual void set(const string &newValue) = 0;

protected:
    int valueType;
};
#endif
