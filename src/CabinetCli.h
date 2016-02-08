#ifndef CABINETCLI_H
#define CABINETCLI_H

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

#include <string>
#include <vector>

using std::string;
using std::vector;

class CabinetCli
{
public:
    CabinetCli();
    CabinetCli(int serverPort);
    CabinetCli(const char *serverIp, int serverPort);
    int readClientInput();
    int formatClientInput();
    int checkClientInput();
    int sendClientInput();
    int receiveServerOutput();
    int displayServerOutput();

private:
    string clientInput;
    string clientInputFormated;
    string serverOutputBuf;
    vector<string> serverOutput;
    int serverOutputArgc;
    int serverOutputArgvLen;
    bool outputReady;
    string serverIp;
    int serverPort;
};

#endif
