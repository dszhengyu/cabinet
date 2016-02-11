#ifndef STRINGOBJ_H
#define STRINGOBJ_H

#include <string>
#include "ValueObj.h"

using std::string;
class ValueObj;

class StringObj : public ValueObj
{
public:
    StringObj();
    StringObj(const string &initValue);
    const string &get() const {return this->stringValue;}
    void set(const string &newValue) {this->stringValue = newValue;}

private:
    string stringValue;
};
#endif
