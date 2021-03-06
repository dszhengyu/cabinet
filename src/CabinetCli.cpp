#include "CabinetCli.h"
#include "Const.h"
#include "Log.h"
#include "Util.h"
#include <iostream>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

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
    this->setPrompt();
    this->commandKeeperPtr = new CommandKeeper();
    this->commandKeeperPtr->createClientCommandMap();
}

void CabinetCli::setPrompt() {
    this->prompt = this->serverIp + string(":") + std::to_string(this->serverPort) + string(">");
}

void CabinetCli::printPrompt() {
    std::cout << this->prompt;
    std::cout.flush();
}

int CabinetCli::connectServer() {
    //logDebug("cabinet cli connect server");
    if ((this->connectFd = Util::connectTcp(this->serverIp.c_str(), this->serverPort)) == CABINET_ERR) {
        std::cout << "connect server error" << std::endl;
        exit(1);
    }
    return CABINET_OK;
}

void CabinetCli::reSetServer(const string &newServerIp, const int newServerPort) {
    this->serverIp = newServerIp;
    this->serverPort = newServerPort;
    close(this->connectFd);
    this->connectServer();
    this->setPrompt();
}

/*
 * notice: loop until user input sth
 */
int CabinetCli::readClientInput() {
    //logDebug("cabinet cli read client input");
    while (this->clientInput.size() == 0) {
        this->resetAll();
        this->printPrompt();
        std::getline(std::cin, this->clientInput);
    }
    return CABINET_OK;
}


int CabinetCli::formatClientInput() {
    //logDebug("cabinet cli format client input");
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
        return CABINET_ERR;
    }
    //check command argc is correct
    int argc = selectedCommand.commandArgc();
    if (argc >= 0) {
        if (((int)this->clientInputSplited.size() - 1) != argc) {
            std::cout << "Command Parameter Number Error" << std::endl;
            this->resetAll();
            return CABINET_ERR;
        }
    }
    else {
        if ((-((int)this->clientInputSplited.size() - 1)) > (argc)) {
            std::cout << "Command Parameter Number Error" << std::endl;
            this->resetAll();
            return CABINET_ERR;
        }
    }
    
    //append command type
    const char commandType = selectedCommand.commandType();
    this->protocolStream.appendCommandType(commandType);
    
    //append rest into protocol stream
    for (auto begin = this->clientInputSplited.begin(); begin != this->clientInputSplited.end(); ++begin) {
        this->protocolStream.appendReplyBody(*begin);
    }

    //clear splited string vector
    this->clientInputSplited.clear();

    return CABINET_OK;
}

int CabinetCli::resetAll() {
    //logDebug("cabinet cli reset all");
    this->clientInput.clear();
    this->clientInputSplited.clear();
    this->protocolStream.clear();
    return CABINET_OK;
}

int CabinetCli::sendClientInput() {
    //logDebug("cabinet cli sending client input");
    //logDebug("cabinet protocol stream send \nbuf[\n%s]\n buf_len[%d]", 
    //        this->protocolStream.getSendBuf().c_str(), 
    //        this->protocolStream.getSendBufLen());
    if (this->protocolStream.getSendBufLen() == 0) {
        this->resetAll();
        return CABINET_ERR;
    }

    while (this->protocolStream.getSendBufLen() != 0) {
        int nWrite = this->protocolStream.getSendBufLen();
        const string &outputBuf = this->protocolStream.getSendBuf();
        nWrite = write(this->connectFd, outputBuf.c_str(), nWrite);
        if (nWrite == -1) {
            std::cout << "Send Command to Server Error" << std::endl;
            this->resetAll();
            return CABINET_ERR;
        }
        //logDebug("cabinet cli send [%d] byte", nWrite);
        this->protocolStream.eraseSendBuf(0, nWrite);
    }
    return CABINET_OK;
}

int CabinetCli::receiveServerOutput() {
    //logDebug("cabinet cli receiving server output");
    while (!this->protocolStream.isReceiveComplete()) {
        //read from server, fill buf
        char readBuf[this->READ_MAX_LEN];
        int nRead = 0;
        //logDebug("cabinet cli reading from server");
        nRead = read(this->connectFd, readBuf, this->READ_MAX_LEN);
        if (nRead == -1) {
            std::cout << "Connect to Server Error" << std::endl;
            this->resetAll();
            return CABINET_ERR;
        }   
        else if (nRead == 0) {
            std::cout << "Server Close Connection" << std::endl;
            exit(1);
            return CABINET_ERR;
        }
        readBuf[nRead] = '\0';
        logDebug("cabinet cli receive server output[\n%s] output_len[%d]", readBuf, nRead);
        this->protocolStream.fillReceiveBuf(readBuf, nRead);

        //resolve receive stream
        if (this->protocolStream.resolveReceiveBuf() == CABINET_ERR) {
            std::cout << "Server Reply Format Error" << std::endl;
            this->resetAll();
            return CABINET_ERR;
        }
    }

    if (this->protocolStream.getCommandName() == "redirect") {
        //logDebug("cabinet cli receive redirect");
        const vector<string> &argv = this->protocolStream.getReceiveArgv();
        const string &newIP = argv[2];
        int newPort;
        try{
            newPort = std::stoi(argv[4]);
        } catch (std::exception &e) {
            logFatal("cabinet cli get new port error, what[%s]", e.what());
            exit(1);
        }
        this->reSetServer(newIP, newPort);
    }
    return CABINET_OK;
}

int CabinetCli::displayServerOutput() {
    //logDebug("cabinet cli display server output");
    for (const string &str : this->protocolStream.getReceiveArgv()) {
        std::cout << str << std::endl; 
    }
    this->resetAll();
    return CABINET_OK;
}
