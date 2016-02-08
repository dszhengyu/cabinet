#ifndef PROTOCOLSTREAM_H
#define PROTOCOLSTREAM_H

#include <string>
#include <vector>
using std::string;
using std::vector;

class ProtocolStream
{
public:
    ProtocolStream(bool hasCommandType);
    int fillInputBuf(const char *str, int strLen);
    int resolveInputBuf();
    const char *getInputBuf() const {return this->inputBuf.c_str();}
    bool isReadyToExecute() const {return this->readyToExecute;}
    const string&getCommandName() const {return this->argv[0];}
    int resetBufForNextCommand();

private:
    bool isInputBufAvaliable() const;
    bool hasCommandType;
    string inputBuf;
    int argc;//argc三种状态: -1, 没有命令;0, 命令解析完成, 等待执行; >0, 命令还在解析中
    vector<string> argv;
    long curArgvLen;
    bool readyToExecute;
    char commandType;
    
};
#endif
