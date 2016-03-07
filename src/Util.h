#ifndef UTIL_H
#define UTIL_H

#include "Const.h"
#include <string>
using std::string;

class Util
{
public:
    static string getCurrentTime();
    static long getCurrentTimeInMs();
    static int setNonBlock(int fd);
    static int listenTcp(int port);
    static int acceptTcp(const int listenfd, string &ip, int &port);
    static int connectTcp(const char *ip, int port);
    static int daemonize();
};

#endif
