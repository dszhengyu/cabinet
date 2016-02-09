#include "CabinetCli.h"
#include "Const.h"
#include "Log.h"
#include <iostream>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

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
    commandKeeperPtr(nullptr),
    connectFd(-1)
{
    this->prompt = this->serverIp + string(":") + std::to_string(this->serverPort) + string(">");
    this->commandKeeperPtr = new CommandKeeper();
    this->commandKeeperPtr->createCommandMap();
}

void CabinetCli::printPrompt() {
    std::cout << this->prompt;
    std::cout.flush();
}

int CabinetCli::connectServer() {
    struct sockaddr_in clientAddr;
    
    if ((this->connectFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "create socket err" << std::endl;
        exit(1);
    }

    bzero(&clientAddr, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(this->serverPort);
    if (inet_pton(AF_INET, this->serverIp.c_str(), &clientAddr.sin_addr) <= 0) {
        std::cout << "create client addr err" << std::endl;
        exit(1);
    }
    
    if (connect(this->connectFd, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) < 0) {
        std::cout << "connect server err" << std::endl;
        exit(1);
    }
    return CABINET_OK;
}

/*
 * notice: loop until user input sth
 */
int CabinetCli::readClientInput() {
    while (this->clientInput.size() == 0) {
        this->resetAll();
        this->printPrompt();
        std::getline(std::cin, this->clientInput);
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
        this->resetAll();
        this->printPrompt();
        return CABINET_ERR;
    }

    //put client input in protocol stream
    this->protocolStream.initReplyHead(this->clientInputSplited.size());

    //find command
    const string &commandName = this->clientInputSplited[0];
    const Command &selectedCommand = this->commandKeeperPtr->selectCommand(commandName);
    //check if command is valid
    if (!selectedCommand.isCommandValid()) {
        std::cout << "Command is Invalid" << std::endl;
        this->resetAll();
        this->printPrompt();
        return CABINET_ERR;
    }
    //check command argc is correct
    int argc = selectedCommand.commandArgc();
    if (argc >= 0) {
        if (((int)this->clientInputSplited.size() - 1) != argc) {
            std::cout << "Command Parameter Error" << std::endl;
            this->resetAll();
            this->printPrompt();
            return CABINET_ERR;
        }
    }
    else {
        if ((-((int)this->clientInputSplited.size() - 1)) > (argc)) {
            std::cout << "Command Parameter Error" << std::endl;
            this->resetAll();
            this->printPrompt();
            return CABINET_ERR;
        }
    }
    
    //append command type
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

int CabinetCli::resetAll() {
    this->clientInput.clear();
    this->clientInputSplited.clear();
    this->protocolStream.clear();
    return CABINET_OK;
}

int CabinetCli::sendClientInput() {
    if (this->protocolStream.getSendBufLen() == 0) {
        this->resetAll();
        this->printPrompt();
        return CABINET_ERR;
    }

    while (this->protocolStream.getSendBufLen() != 0) {
        int nWrite = this->protocolStream.getSendBufLen();
        const string &outputBuf = this->protocolStream.getSendBuf();
        nWrite = write(this->connectFd, outputBuf.c_str(), nWrite);
        if (nWrite == -1) {
            std::cout << "Send Command to Server Error" << std::endl;
            this->resetAll();
            this->printPrompt();
            return CABINET_ERR;
        }
        this->protocolStream.eraseSendBuf(0, nWrite);
    }
    return CABINET_OK;
}

int CabinetCli::receiveServerOutput() {
    while (!this->protocolStream.isReceiveComplete()) {
        //read from server, fill buf
        char readBuf[this->READ_MAX_LEN];
        int nRead = 0;
        nRead = read(this->connectFd, readBuf, this->READ_MAX_LEN);
        if (nRead == -1) {
            std::cout << "Connect to Server Error" << std::endl;
            this->resetAll();
            this->printPrompt();
            return CABINET_ERR;
        }   
        else if (nRead == 0) {
            std::cout << "Server Close Connection" << std::endl;
            this->resetAll();
            this->printPrompt();
            return CABINET_ERR;
        }
        this->protocolStream.fillReceiveBuf(readBuf, nRead);

        //resolve receive stream
        if (this->protocolStream.resolveReceiveBuf() == CABINET_ERR) {
            std::cout << "Server Reply Format Error" << std::endl;
            this->resetAll();
            this->printPrompt();
            return CABINET_ERR;
        }
    }
    return CABINET_OK;
}

int CabinetCli::displayServerOutput() {
    for (const string &str : this->protocolStream.getReceiveArgv()) {
        std::cout << str << std::endl; 
    }
    this->resetAll();
    this->printPrompt();
    return CABINET_OK;
}
