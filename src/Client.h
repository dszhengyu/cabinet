#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <vector>
#include "CommandKeeper.h"

using std::string;
using std::vector;

class Client
{
public:
    Client(long clientId, CommandKeeper *commandKeeperPtr);
    void readBuff();
    void sendReply();
    void resolveCommand();

private:
    long clientId;
    string inputBuf;
    string outputBuf;
    int argc;
    vector<string> argv;
    int curArgvLen;
    int curArgc;
    CommandKeeper *commandKeeperPtr;
    //EventPoll *eventPoll;
};
#endif
