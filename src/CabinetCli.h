#ifndef CABINETCLI_H
#define CABINETCLI_H

#include <string>
#include <vector>
#include "ProtocolStream.h"
#include "CommandKeeper.h"

using std::string;
using std::vector;

class CabinetCli
{
public:
    CabinetCli(const char *serverIp, int serverPort);
    int readClientInput();
    int formatClientInput();
    int sendClientInput();
    int receiveServerOutput();
    int displayServerOutput();
    int connectServer();

private:
    void setPrompt();
    void printPrompt();
    void reSetServer(const string &newServerIp, const int newServerPort);
    int resetAll();
    string serverIp;
    int serverPort;
    string clientInput;
    vector<string> clientInputSplited;
    string serverOutputBuf;
    vector<string> serverOutput;
    int serverOutputArgc;
    int serverOutputArgvLen;
    bool outputReady;
    string prompt;
    ProtocolStream protocolStream;
    CommandKeeper *commandKeeperPtr;
    const int READ_MAX_LEN = 1024 * 16;
    int connectFd;
};

#endif
