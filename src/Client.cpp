#include "Client.h"

Client::Client(long clientId, CommandKeeper *commandKeeperPtr):
    clientId(clientId),
    inputBuf(),
    outputBuf(),
    argc(-1),
    argv(),
    curArgvLen(-1),
    curArgc(-1),
    commandKeeperPtr(commandKeeperPtr)
{

}

