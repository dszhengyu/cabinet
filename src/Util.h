#ifndef UTIL_H
#define UTIL_H

#include "Const.h"
#include <string>
using std::string;

class Util
{
public:
    static string getCurrentTime();
    static int setNonBlock(int fd);
};

#endif
