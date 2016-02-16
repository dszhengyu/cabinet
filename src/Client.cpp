#include "Client.h"
#include "Const.h"
#include <unistd.h>


Client::Client(long clientId, CommandKeeper *commandKeeper, int fd, EventPoll *eventPoll, DataBase *dataBasePtr, PersistenceFile *pf):
    clientId(clientId),
    commandKeeper(commandKeeper),
    fd(fd),
    eventPoll(eventPoll),
    dataBasePtr(dataBasePtr),
    protocolStream(true),
    category(Client::NORMAL_CLIENT),
    pf(pf)
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
            logWarning("read from client client_id[%d] error", this->getClientId());
            return CABINET_ERR;
        }
    }   
    else if (nRead == 0) {
        logNotice("client client_id[%d] close connection", this->getClientId());
        return CABINET_ERR;
    }

    return this->protocolStream.fillReceiveBuf(readBuf, nRead);
}

int Client::fillReceiveBuf(const string &str) {
    return this->protocolStream.fillReceiveBuf(str);
}

int Client::resolveReceiveBuf() {
    if (this->protocolStream.resolveReceiveBuf() == CABINET_ERR) {
        logWarning("client client_id[%d] input format error, input_buf[%s]", 
                this->getClientId(), this->protocolStream.getReceiveBuf());
        return CABINET_ERR;
    }
    return CABINET_OK;
}

/* 
 * brief: 当输入缓冲区中当前命令解析完毕时调用
 * pf: when it is not a pf client and pf is require, pf need pf command
 *
 */
int Client::executeCommand() {
    const string &commandName = this->protocolStream.getCommandName();
    Command &selectedCommand = this->commandKeeper->selectCommand(commandName);

    //do pf
    if ((this->category != Client::LOCAL_PF_CLIENT) && 
            (pf != nullptr) &&
            (selectedCommand.needPF() == true)) {
        this->pf->appendToPF(this);
    }

    //execute
    if (selectedCommand(this) == CABINET_ERR) {
        logWarning("client client_id[%d] execute command error", this->getClientId());
        return CABINET_ERR;
    }
    return CABINET_OK;
}

int Client::initReplyHead(int argc) {
    logDebug("init reply head");
    if (this->category != NORMAL_CLIENT) {
        logDebug("no normal client no reply");
        return CABINET_OK; 
    }
    //在eventloop中删掉可读, 安装可写事件
    this->eventPoll->removeFileEvent(this, READ_EVENT);
    this->eventPoll->createFileEvent(this, WRITE_EVENT);

    this->protocolStream.initReplyHead(argc);
    return CABINET_OK;
}

int Client::appendReplyBody(const string &part) {
    logDebug("append reply body");
    if (this->category != NORMAL_CLIENT) {
        logDebug("no normal client no reply");
        return CABINET_OK; 
    }
    this->protocolStream.appendReplyBody(part);
    return CABINET_OK;
}

/* 
 * brief: 当输出缓冲区中有数据需要发送给客户时调用
 * notice: 发送完成后, 需要删除eventloop中的可写轮询, 安装可读轮询
 */
int Client::sendReply() {
    logDebug("sending reply");
    if (this->category != NORMAL_CLIENT) {
        logDebug("no normal client no reply");
        return CABINET_OK; 
    }
    logDebug("protocol stream send \nbuf[\n%s]\n buf_len[%d]", 
            this->protocolStream.getSendBuf().c_str(), 
            this->protocolStream.getSendBufLen());
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
            logWarning("write data to client client_id[%d] error", this->getClientId());
            return CABINET_ERR;
        }
    }
    logDebug("send [%d] byte", nWrite);
    this->protocolStream.eraseSendBuf(0, nWrite);
    if (this->protocolStream.getSendBufLen() == 0) {
        //delete file event loop for write
        if (this->eventPoll->removeFileEvent(this, WRITE_EVENT) == CABINET_ERR) {
            logWarning("delete client client_id[%d] write file event error", this->getClientId());
            return CABINET_ERR;
        }
        //install file event loop for read
        if (this->eventPoll->createFileEvent(this, READ_EVENT) == CABINET_ERR) {
            logWarning("create client client_id[%d] read file event error", this->getClientId());
            return CABINET_ERR;
        }
        this->protocolStream.clear();
    }
    return CABINET_OK;
}

Client::~Client() {
    close(this->fd);
}
