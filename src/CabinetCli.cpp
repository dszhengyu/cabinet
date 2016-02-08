#include "CabinetCli.h"
#include "Const.h"
#include "Log.h"
#include <iostream>

CabinetCli::CabinetCli():
  CabinetCli(SERVER_PORT)
{
}

CabinetCli::CabinetCli(int serverPort):
    CabinetCli(SERVER_IP, serverPort)
{
}

CabinetCli::CabinetCli(const char *serverIp, int serverPort) :
    serverIp(serverIp),
    serverPort(serverPort),
    clientInput(),
    clientInputSplited(),
    serverOutputBuf(),
    serverOutput(),
    serverOutputArgc(-1),
    serverOutputArgvLen(-1),
    outputReady(false),
    prompt(),
    protocolStream(false),
    commandKeeperPtr(nullptr)
{
    this->commandKeeperPtr = new CommandKeeper();
    this->commandKeeperPtr->createCommandMap();
}

/*
 * notice: when user input nothing, return error, and caller should loop
 */
int CabinetCli::readClientInput() {
    std::getline(std::cin, this->clientInput);
    if (this->clientInput.size() == 0) {
        this->printPrompt();
        this->resetAll();
        return CABINET_ERR;
    }
    return CABINET_OK;
}


int CabinetCli::formatClientInput() {
    //split the client input into string vector
    while (this->clientInput.length() != 0) {
        size_t blankPosition = this->clientInput.find_first_of(' ');
        if (blankPosition == string::npos) {
            this->clientInputSplited.push_back(this->clientInput);
            this->clientInput.clear();
            break;
        }
        this->clientInputSplited.push_back(string(this->clientInput, 0, blankPosition));
        size_t noBlankPosition = this->clientInput.find_first_not_of(' ', blankPosition);
        if (noBlankPosition == string::npos) {
            this->clientInput.clear();
            break;
        }
        this->clientInput.erase(0, noBlankPosition);
    }
    this->clientInput.clear();
    if (this->clientInputSplited.size() == 0) {
        this->printPrompt();
        this->resetAll();
        return CABINET_ERR;
    }

    //put client input in protocol stream
    this->protocolStream.initReplyHead(this->clientInputSplited.size());

    //find command type
    const string &commandName = this->clientInputSplited[0];
    const Command &selectedCommand = this->commandKeeperPtr->selectCommand(commandName);
    const char commandType = selectedCommand.commandType();
    this->protocolStream.appendCommandType(commandType);
    
    //append rest into protocol stream
    for (auto begin = this->clientInputSplited.begin() + 1; begin != this->clientInputSplited.end(); ++begin) {
        this->protocolStream.appendReplyBody(*begin);
    }

    //clear splited string vector
    this->clientInputSplited.clear();

    return CABINET_OK;
}

void CabinetCli::printPrompt() {
    std::cout << this->prompt;
    std::cout.flush();
}

int CabinetCli::resetAll() {
    clientInput.clear();
    clientInputSplited.clear();
    //protocolStream
    return CABINET_OK;
}
//int CabinetCli::checkClientInput();
//int CabinetCli::sendClientInput();
//int CabinetCli::receiveServerOutput();
//int CabinetCli::displayServerOutput();
