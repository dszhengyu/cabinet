#include "CabinetCli.h"
#include "Const.h"

CabinetCli::CabinetCli()
{
    CabinetCli(SERVER_PORT);
}

CabinetCli::CabinetCli(int serverPort)
{
    CabinetCli(SERVER_IP, serverPort);
}

CabinetCli::CabinetCli(const char *serverIp, int serverPort) :
    serverIp(serverIp),
    serverPort(serverPort),
    clientInput(),
    clientInputFormated(),
    serverOutputBuf(),
    serverOutput(),
    serverOutputArgc(-1),
    serverOutputArgvLen(-1),
    outputReady(false),
    prompt()
{

}
