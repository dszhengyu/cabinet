#include "ProtocolStream.h"
#include "Const.h"
#include "Log.h"

ProtocolStream::ProtocolStream(bool hasCommandType):
    hasCommandType(hasCommandType)
{
}

int ProtocolStream::resetBufForNextCommand() {
    this->argc = -1;
    this->commandType = '\0';
    this->curArgvLen = -1;
    this->argv.clear();
    this->readyToExecute = false;
    return CABINET_OK;
}

int ProtocolStream::fillInputBuf(const char *str, int strLen) {
    this->inputBuf.append(str, strLen);
    return CABINET_OK;    
}

bool ProtocolStream::isInputBufAvaliable() const {
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
int ProtocolStream::resolveInputBuf() {
    if (!this->isInputBufAvaliable()) {
        return CABINET_OK;    
    }

    if (this->argc == -1) {
        //开始解析新请求
        if (this->inputBuf[0] != '*') {
            Log::warning("protocol stream input format error, input_buf[%s]", this->inputBuf.c_str());
            return CABINET_ERR;
        }

        size_t firstLF = this->inputBuf.find_first_of('\n', 1);
        if (firstLF == string::npos) {
            return CABINET_OK;    
        }
        this->argc = stoi(string(this->inputBuf, 1, firstLF));
        if (this->argc <= 0) {
            Log::warning("protocol stream input format error, input_buf[%s]", this->inputBuf.c_str());
            return CABINET_ERR;
        }
        this->inputBuf.erase(0, firstLF + 1);
        if (!this->isInputBufAvaliable()) {
            return CABINET_OK;    
        }
    }

    if (this->hasCommandType && this->commandType == '\0') {
        //开始解析请求属性
        if (this->inputBuf[0] != '#') {
            Log::warning("protocol stream input format error, input_buf[%s]", this->inputBuf.c_str());
            return CABINET_ERR;
        }

        size_t firstLF = this->inputBuf.find_first_of('\n', 1);
        if (firstLF == string::npos) {
            return CABINET_OK;    
        }
        if (firstLF != 2) {
            Log::warning("protocol stream input format error, input_buf[%s]", this->inputBuf.c_str());
            return CABINET_ERR;
        }
        this->commandType = this->inputBuf[1];
        this->inputBuf.erase(0, firstLF + 1);
        if (!this->isInputBufAvaliable()) {
            return CABINET_OK;    
        }
    }

    //开始解析参数
    while (this->isInputBufAvaliable()) {
        size_t firstLF = this->inputBuf.find_first_of('\n');
        if (this->curArgvLen == -1) {
            //当前参数长度未解析
            if (this->inputBuf[0] != '$') {
                Log::warning("protocol stream input format error, input_buf[%s]", this->inputBuf.c_str());
                return CABINET_ERR;
            }
            this->curArgvLen = stoi(string(this->inputBuf, 1, firstLF));
            this->inputBuf.erase(0, firstLF + 1);
            continue;
        }
        else {
            //按照长度获取当前参数
            if ((long)firstLF != this->curArgvLen) {
                Log::warning("protocol stream input format error, input_buf[%s]", this->inputBuf.c_str());
                return CABINET_ERR;
            }
            this->argv.push_back(string(this->inputBuf, 0, firstLF));
            this->curArgvLen = -1;
            this->inputBuf.erase(0, firstLF + 1);
            continue;
            --this->argc;
            if (this->argc == 0) {
                this->readyToExecute = true;
                break;
            }
        }
    }

    return CABINET_OK;
}

