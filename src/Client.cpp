#include "Client.h"
#include "Const.h"
#include <unistd.h>

Client::Client(long clientId, CommandKeeper *commandKeeperPtr, int fd):
    clientId(clientId),
    inputBuf(),
    outputBuf(),
    argc(-1),
    argv(),
    curArgvLen(-1),
    curArgc(-1),
    commandKeeperPtr(commandKeeperPtr),
    fd(fd),
    readyToExecute(0),
    commandType('\0')
{

}

/* 
 * brief: 当客户端对应套接字可读时调用
 *      将接收到的内容放入输入缓冲区
 * notice: 用户段关闭或者读取异常时, 返回错误, 调用者需要删除该客户端
 */
int Client::fillInputBuf() {
    char readBuf[this->READ_MAX_LEN];
    int nRead = 0;
    nRead = read(this->fd, readBuf, this->READ_MAX_LEN);
    if (nRead == -1) {
        if (errno == EWOULDBLOCK) {
            return CABINET_OK;
        }
        else {
            Log::warning("read from client client_id[%d] error", this->getClientId());
            return CABINET_ERR;
        }
    }   
    else if (nRead == 0) {
        Log::notice("client client_id[%d] close connection", this->getClientId());
        return CABINET_ERR;
    }

    this->inputBuf.append(readBuf, nRead);
    return CABINET_OK;
}

/* 
 * brief: 当输入缓冲区中有可解析的内容时调用
 *      解析客户端输入缓冲区的内容
 * notice: 失败返回 错误, 需要清除客户端及其连接
 * format: 字节流的格式为: 
 *      *2\n#w$3\nget\n$3\nfoo\n
 *      星号 参数个数 LF #号 命令属性(读或写) LF \
 *      $美元 第一个参数字节数 LF 第一个参数 LF \
 *      $美元 第二个参数字节数 LF 第二个参数 LF
 *
 * argc三种状态: -1, 没有命令;0, 命令解析完成, 等待执行; >0, 命令还在解析中
 *
 * warning: 时刻注意, 缓冲区中不一定包含足够的内容, 内容不足时, 不破坏状态
 *
 */
int Client::resolveInputBuf() {
    if (this->inputBuf.length() == 0) {
        return CABINET_OK;    
    }

    if (this->argc == -1) {
        //开始解析新请求
        if (this->inputBuf[0] != '*') {
            Log::warning("client client_id[%d] input format error, input_buf[%s]", this->getClientId(), this->inputBuf.c_str());
            return CABINET_ERR;
        }

        size_t firstCR = this->inputBuf.find_first_of('\n', 1);
        if (firstCR == string::npos) {
            return CABINET_OK;    
        }
        this->argc = stoi(string(this->inputBuf, 1, firstCR));
        this->curArgc = 0;
        this->inputBuf.erase(0, firstCR + 1);
    }
    if (this->inputBuf.length() == 0) {
        return CABINET_OK;    
    }

    if (this->commandType == '\0') {
        //开始解析请求属性
        if (this->inputBuf[0] != '#') {
            Log::warning("client client_id[%d] input format error, input_buf[%s]", this->getClientId(), this->inputBuf.c_str());
            return CABINET_ERR;
        }

        size_t firstCR = this->inputBuf.find_first_of('\n', 1);
        if (firstCR == string::npos) {
            return CABINET_OK;    
        }
        if (firstCR != 2) {
            Log::warning("client client_id[%d] input format error, input_buf[%s]", this->getClientId(), this->inputBuf.c_str());
            return CABINET_ERR;
        }
        this->commandType = this->inputBuf[1];
        this->inputBuf.erase(0, firstCR + 1);
    }
    if (this->inputBuf.length() == 0) {
        return CABINET_OK;    
    }

    //开始解析参数
    //to-do


    return CABINET_OK;
}

/* 
 * brief: 当输入缓冲区中当前命令解析完毕时调用
 */
int Client::executeCommand() {

    return CABINET_OK;
}
