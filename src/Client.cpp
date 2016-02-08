#include "Client.h"
#include "Const.h"
#include <unistd.h>


Client::Client(long clientId, CommandKeeper *commandKeeper, int fd, EventPoll *eventPoll):
    clientId(clientId),
    commandKeeper(commandKeeper),
    fd(fd),
    eventPoll(eventPoll),
    protocolStream(true)
{

}

/* 
 * brief: 当客户端对应套接字可读时调用
 *      将接收到的内容放入输入缓冲区
 * notice: 用户段关闭或者读取异常时, 返回错误, 调用者需要删除该客户端
 */
int Client::fillReceiveBuf() {
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

    return this->protocolStream.fillReceiveBuf(readBuf, nRead);
}

int Client::resolveReceiveBuf() {
    if (this->protocolStream.resolveReceiveBuf() == CABINET_ERR) {
        Log::warning("client client_id[%d] input format error, input_buf[%s]", 
                this->getClientId(), this->protocolStream.getReceiveBuf());
        return CABINET_ERR;
    }
    return CABINET_OK;
}

/* 
 * brief: 当输入缓冲区中当前命令解析完毕时调用
 */
int Client::executeCommand() {
    const string &commandName = this->protocolStream.getCommandName();
    Command &selectedCommand = this->commandKeeper->selectCommand(commandName);
    if (selectedCommand(*this) == CABINET_ERR) {
        Log::warning("client client_id[%d] execute command error", this->getClientId());
        return CABINET_ERR;
    }
    this->protocolStream.resetBufForNextCommand();

    return CABINET_OK;
}

int Client::initReplyHead(int argc) {
    //在eventloop中安装可写事件
    eventPoll->createFileEvent(this, WRITE_EVENT);

    this->protocolStream.initReplyHead(argc);
    return CABINET_OK;
}

int Client::appendReplyBody(const string &part) {
    //在eventloop中安装可写事件
    eventPoll->createFileEvent(this, WRITE_EVENT);

    this->protocolStream.appendReplyBody(part);
    return CABINET_OK;
}

/* 
 * brief: 当输出缓冲区中有数据需要发送给客户时调用
 * notice: 发送完成后, 需要删除eventloop中的可写轮询
 */
int Client::sendReply() {
    if (this->protocolStream.getSendBufLen() == 0) {
        return CABINET_OK;
    }

    int nWrite = this->protocolStream.getSendBufLen();
    const string &outputBuf = this->protocolStream.getSendBuf();
    nWrite = write(this->fd, outputBuf.c_str(), nWrite);
    if (nWrite == -1) {
        if (errno == EWOULDBLOCK) {
            return CABINET_OK;
        }
        else {
            Log::warning("write data to client client_id[%d] error", this->getClientId());
            return CABINET_ERR;
        }
    }
    this->protocolStream.eraseSendBuf(0, nWrite);
    if (this->protocolStream.getSendBufLen() == 0) {
        //delete file event loop for write
        if (eventPoll->removeFileEvent(this, WRITE_EVENT) == CABINET_ERR) {
            Log::warning("delete client client_id[%d] write file event error", this->getClientId());
            return CABINET_ERR;
        }
    }
    return CABINET_OK;
}
