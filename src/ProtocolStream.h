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
    ProtocolStream(bool hasCommandType, bool wrapDefaultCommand, string wrapCommandName);
    //input section
    int fillReceiveBuf(const char *str, int strLen);
    int fillReceiveBuf(const string &str);
    int resolveReceiveBuf();
    const char * getReceiveBuf() const {return this->inputBuf.c_str();}
    bool isReceiveComplete() const {return this->receiveComplete;}
    const string &getCommandName() const {return this->argv[0];}
    const vector<string> &getReceiveArgv() const {return this->argv;}
    const string &getCurCommandBuf() const {return this->curCommandBuf;}
    int clear();
    //output section
    int initReplyHead(int argc);
    int appendReplyBody(const string &);
    int appendCommandType(const char commandType);
    const string &getSendBuf() const {return this->outputBuf;}
    size_t getSendBufLen() const {return this->outputBuf.length();}
    void eraseSendBuf(size_t pos, size_t len) {this->outputBuf.erase(pos, len);}
    int fillSendBuf(const string &str);

private:
    bool isReceiveBufAvaliable() const;
    bool hasCommandType;
    string inputBuf;
    string curCommandBuf;
    int argc;//argc三种状态: -1, 没有命令;0, 命令解析完成, 等待执行; >0, 命令还在解析中
    vector<string> argv;
    long curArgvLen;
    bool receiveComplete;
    char commandType;
    string outputBuf;
    bool wrapDefaultCommand;
    string wrapCommandName;
};
#endif
