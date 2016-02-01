#include "Client.h"
#include "Const.h"
#include <unistd.h>

using std::stoi;
using std::to_string;

Client::Client(long clientId, CommandKeeper *commandKeeperPtr, int fd):
    clientId(clientId),
    inputBuf(),
    outputBuf(),
    argc(-1),
    argv(),
    curArgvLen(-1),
    commandKeeperPtr(commandKeeperPtr),
    fd(fd),
    readyToExecute(false),
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

int Client::isInputBufAvaliable() const {
    if (this->inputBuf.length() != 0 &&
            this->inputBuf.find_first_of('\n') != string::npos) {
        return true;
    }
    return false;
}

int Client::resetBufForNextCommand() {
    this->argc = -1;
    this->commandType = '\0';
    this->curArgvLen = -1;
    this->argv.clear();
    this->readyToExecute = false;
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
    if (!this->isInputBufAvaliable()) {
        return CABINET_OK;    
    }

    if (this->argc == -1) {
        //开始解析新请求
        if (this->inputBuf[0] != '*') {
            Log::warning("client client_id[%d] input format error, input_buf[%s]", this->getClientId(), this->inputBuf.c_str());
            return CABINET_ERR;
        }

        size_t firstLF = this->inputBuf.find_first_of('\n', 1);
        if (firstLF == string::npos) {
            return CABINET_OK;    
        }
        this->argc = stoi(string(this->inputBuf, 1, firstLF));
        if (this->argc == 0) {
            Log::warning("client client_id[%d] input format error, input_buf[%s]", this->getClientId(), this->inputBuf.c_str());
            return CABINET_ERR;
        }
        this->inputBuf.erase(0, firstLF + 1);
        if (!this->isInputBufAvaliable()) {
            return CABINET_OK;    
        }
    }

    if (this->commandType == '\0') {
        //开始解析请求属性
        if (this->inputBuf[0] != '#') {
            Log::warning("client client_id[%d] input format error, input_buf[%s]", this->getClientId(), this->inputBuf.c_str());
            return CABINET_ERR;
        }

        size_t firstLF = this->inputBuf.find_first_of('\n', 1);
        if (firstLF == string::npos) {
            return CABINET_OK;    
        }
        if (firstLF != 2) {
            Log::warning("client client_id[%d] input format error, input_buf[%s]", this->getClientId(), this->inputBuf.c_str());
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
                Log::warning("client client_id[%d] input format error, input_buf[%s]", this->getClientId(), this->inputBuf.c_str());
                return CABINET_ERR;
            }
            this->curArgvLen = stoi(string(this->inputBuf, 1, firstLF));
            this->inputBuf.erase(0, firstLF + 1);
            continue;
        }
        else {
            //按照长度获取当前参数
            if ((long)firstLF != this->curArgvLen) {
                Log::warning("client client_id[%d] input format error, input_buf[%s]", this->getClientId(), this->inputBuf.c_str());
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

/* 
 * brief: 当输入缓冲区中当前命令解析完毕时调用
 */
int Client::executeCommand() {
    if (this->argv.size() == 0) {
        Log::warning("client client_id[%d] executing the command but argv is empty!", this->getClientId());
        return CABINET_ERR;
    }
    const string &commandName = this->argv[0];
    Command &selectedCommand = this->commandKeeperPtr->selectCommand(commandName);
    if (selectedCommand(*this) == CABINET_ERR) {
        Log::warning("client client_id[%d] execute command error", this->getClientId());
        return CABINET_ERR;
    }

    return CABINET_OK;
}

int Client::initReplyHead(int argc) {
    //to-do 在eventloop中安装可写事件
    this->outputBuf.append("*");
    this->outputBuf.append(to_string(argc));
    this->outputBuf.append("\n");
    return CABINET_OK;
}

int Client::appendReplyBody(const string &part) {
    //to-do 在eventloop中安装可写事件
    this->outputBuf.append("$");
    this->outputBuf.append(to_string(part.length()));
    this->outputBuf.append("\n");
    this->outputBuf.append(part);
    this->outputBuf.append("\n");
    return CABINET_OK;
}

/* 
 * brief: 当输出缓冲区中有数据需要发送给客户时调用
 * notice: 发送完成后, 需要删除eventloop中的可写轮询
 */
int Client::sendReply() {
    if (this->outputBuf.length() == 0) {
        return CABINET_OK;
    }

    int nWrite = this->outputBuf.length();
    nWrite = write(this->fd, this->outputBuf.c_str(), nWrite);
    if (nWrite == -1) {
        if (errno == EWOULDBLOCK) {
            return CABINET_OK;
        }
        else {
            Log::warning("write data to client cliend_id[%d] error", this->getClientId());
            return CABINET_ERR;
        }
    }
    this->outputBuf.erase(0, nWrite);
    if (this->outputBuf.length() == 0) {
        //to-do delete file event loop for write
    }
    return CABINET_OK;
}
