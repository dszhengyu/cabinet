#include "ProtocolStream.h"
#include "Const.h"
#include "Log.h"
using std::stoi;
using std::to_string;

ProtocolStream::ProtocolStream(bool hasCommandType):
    hasCommandType(hasCommandType),
    inputBuf(),
    curCommandBuf(),
    argc(-1),
    argv(),
    curArgvLen(-1),
    receiveComplete(false),
    commandType('\0'),
    outputBuf()
{
}

int ProtocolStream::clear() {
    this->inputBuf.clear();
    this->curCommandBuf.clear();
    this->argc = -1;
    this->argv.clear();
    this->curArgvLen = -1;
    this->receiveComplete = false;
    this->commandType = '\0';
    this->outputBuf.clear();
    return CABINET_OK;
}

int ProtocolStream::fillReceiveBuf(const char *str, int strLen) {
    this->inputBuf.append(str, strLen);
    return CABINET_OK;    
}

int ProtocolStream::fillReceiveBuf(const string &str) {
    this->inputBuf.append(str);
    return CABINET_OK;    
}

bool ProtocolStream::isReceiveBufAvaliable() const {
    if (this->inputBuf.length() != 0 &&
            this->inputBuf.find_first_of('\n') != string::npos) {
        return true;
    }
    return false;
}

/* 
 * brief: 当输入缓冲区中有可解析的内容时调用
 *      解析输入缓冲区的内容
 * notice: 失败返回 错误
 * format: 字节流的格式为: 
 *      *2\n[#w\n]$3\nget\n$3\nfoo\n
 *      星号 参数个数 LF [#号 命令属性(读或写) LF] \
 *      $美元 第一个参数字节数 LF 第一个参数 LF \
 *      $美元 第二个参数字节数 LF 第二个参数 LF
 *
 * argc三种状态: -1, 没有命令;0, 命令解析完成, 等待执行; >0, 命令还在解析中
 *
 * warning: 时刻注意, 缓冲区中不一定包含足够的内容, 内容不足时, 不破坏状态
 *
 */
int ProtocolStream::resolveReceiveBuf() {
    if (!this->isReceiveBufAvaliable()) {
        return CABINET_OK;    
    }

    if (this->argc == -1) {
        //logDebug("searching for *");
        //开始解析新请求
        if (this->inputBuf[0] != '*') {
            logWarning("protocol stream input format error, missing *, input_buf[%s]", this->inputBuf.c_str());
            return CABINET_ERR;
        }

        size_t firstLF = this->inputBuf.find_first_of('\n', 1);
        if (firstLF == string::npos) {
            return CABINET_OK;    
        }
        this->argc = stoi(string(this->inputBuf, 1, firstLF));
        if (this->argc <= 0) {
            logWarning("protocol stream input format error, argc <= 0, input_buf[%s]", this->inputBuf.c_str());
            return CABINET_ERR;
        }
        this->curCommandBuf.append(this->inputBuf, 0, firstLF + 1);
        this->inputBuf.erase(0, firstLF + 1);
        if (!this->isReceiveBufAvaliable()) {
            return CABINET_OK;    
        }
        //logDebug("argc get. argc[%d]", this->argc);
    }

    if (this->hasCommandType && this->commandType == '\0') {
        //logDebug("searching for #");
        //开始解析请求属性
        if (this->inputBuf[0] != '#') {
            logWarning("protocol stream input format error, missing #(command type), input_buf[%s]", this->inputBuf.c_str());
            return CABINET_ERR;
        }

        size_t firstLF = this->inputBuf.find_first_of('\n', 1);
        if (firstLF == string::npos) {
            return CABINET_OK;    
        }
        if (firstLF != 2) {
            logWarning("protocol stream input format error, command type format error, input_buf[%s]", this->inputBuf.c_str());
            return CABINET_ERR;
        }
        this->commandType = this->inputBuf[1];
        this->curCommandBuf.append(this->inputBuf, 0, firstLF + 1);
        this->inputBuf.erase(0, firstLF + 1);
        if (!this->isReceiveBufAvaliable()) {
            return CABINET_OK;    
        }
        //logDebug("command type get command type[%c]", this->commandType);
    }

    //开始解析参数
    while (this->isReceiveBufAvaliable()) {
        size_t firstLF = this->inputBuf.find_first_of('\n');
        if (this->curArgvLen == -1) {
            //logDebug("searching for argv len");
            //当前参数长度未解析
            if (this->inputBuf[0] != '$') {
                logWarning("protocol stream input format error, missing $, input_buf[%s]", this->inputBuf.c_str());
                return CABINET_ERR;
            }
            this->curArgvLen = stoi(string(this->inputBuf, 1, firstLF));
            this->curCommandBuf.append(this->inputBuf, 0, firstLF + 1);
            this->inputBuf.erase(0, firstLF + 1);
            //logDebug("argv len get, argv_len[%d]", this->curArgvLen);
            continue;
        }
        else {
            //按照长度获取当前参数
            //logDebug("searching for argv");
            if ((long)firstLF != this->curArgvLen) {
                logWarning("protocol stream input format error, missing argv, input_buf[%s]", this->inputBuf.c_str());
                return CABINET_ERR;
            }
            this->argv.push_back(string(this->inputBuf, 0, firstLF));
            this->curArgvLen = -1;
            this->curCommandBuf.append(this->inputBuf, 0, firstLF + 1);
            this->inputBuf.erase(0, firstLF + 1);
            //logDebug("argv get, argv[%s]", (this->argv.end() - 1)->c_str());
            --this->argc;
            if (this->argc == 0) {
                this->receiveComplete = true;
                break;
            }
            continue;
        }
    }

    return CABINET_OK;
}

int ProtocolStream::initReplyHead(int argc) {
    //logDebug("start init reply head, argc[%d], \nsend_buf\n[\n%s\n]", argc, this->outputBuf.c_str());
    this->outputBuf.push_back('*');
    this->outputBuf.append(to_string(argc));
    this->outputBuf.push_back('\n');
    //logDebug("end init reply head, argc[%d], \nsend_buf\n[\n%s\n]", argc, this->outputBuf.c_str());
    return CABINET_OK;
}

int ProtocolStream::appendCommandType(const char commandType) {
    //logDebug("start append command type, command type[%c], \nsend_buf\n[\n%s\n]", commandType, this->outputBuf.c_str());
    this->outputBuf.push_back('#');
    this->outputBuf.push_back(commandType);
    this->outputBuf.push_back('\n');
    //logDebug("end append command type, command type[%c], \nsend_buf\n[\n%s\n]", commandType, this->outputBuf.c_str());
    return CABINET_OK;
}

int ProtocolStream::appendReplyBody(const string &part) {
    //logDebug("start append reply body, body_content[%s], \nsend_buf\n[\n%s\n]", part.c_str(), this->outputBuf.c_str());
    this->outputBuf.push_back('$');
    this->outputBuf.append(to_string(part.length()));
    this->outputBuf.push_back('\n');
    this->outputBuf.append(part);
    this->outputBuf.push_back('\n');
    //logDebug("end append reply body, body_content[%s], \nsend_buf\n[\n%s\n]", part.c_str(), this->outputBuf.c_str());
    return CABINET_OK;
}


