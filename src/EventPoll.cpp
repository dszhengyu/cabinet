#include "EventPoll.h"
#include "Const.h"
#include <sys/epoll.h>
#include <unistd.h>

EventPoll::EventPoll(Server *server):
    server(server),
    readFileEventMap(),
    writeFileEventMap()
{

}

int EventPoll::initEventPoll() {
    int eventPollFd = epoll_create(1024);
    if (eventPollFd == -1) {
        Log::fatal("create epoll error");
        return CABINET_ERR;
    }
    this->eventPollFd = eventPollFd;
    return CABINET_OK;
}

/*
 * brief: 添加文件事件, 在epoll中添加, 在map中也需要添加
 */
int EventPoll::createFileEvent(Client *client, int eventType) {
    //alias the true map to edit
    fileeventmap_t *mapToEdit = nullptr;
    if (eventType == READ_EVENT) {
         mapToEdit = &this->readFileEventMap;
    }
    else {
         mapToEdit = &this->writeFileEventMap;
    }

    //find if the event is already create
    if (mapToEdit->find(client->getClientFd()) != mapToEdit->end()) {
        return CABINET_OK;
    }

    //modify epoll
    this->fileEventOperation(client, eventType, this->DEL_EVENT);

    //modify map
    (*mapToEdit)[client->getClientFd()] = client;

    return CABINET_OK;
}

/*
 * brief: remove file event both in epoll and map
 */
int EventPoll::removeFileEvent(Client *client, int eventType) {
    //alias the true map to edit
    fileeventmap_t *mapToEdit = nullptr;
    if (eventType == READ_EVENT) {
         mapToEdit = &this->readFileEventMap;
    }
    else {
         mapToEdit = &this->writeFileEventMap;
    }

    //find if the event is already delete
    if (mapToEdit->find(client->getClientFd()) == mapToEdit->end()) {
        Log::warning("delete file event twice, client_id[%d]", client->getClientId());
        return CABINET_ERR;
    }

    this->fileEventOperation(client, eventType, this->DEL_EVENT);
    
    mapToEdit->erase(client->getClientFd());

    return CABINET_OK;
}

int EventPoll::fileEventOperation(Client *client, int eventType, int opType) {
    int op = ((opType == this->ADD_EVENT) ? EPOLL_CTL_ADD : EPOLL_CTL_DEL);
    int pollEvent = ((eventType == READ_EVENT) ? EPOLLIN : EPOLLOUT);
    int clientFd = client->getClientFd();
    struct epoll_event ee;
    ee.events = pollEvent;
    ee.data.fd = clientFd;
    epoll_ctl(this->eventPollFd, op, clientFd, &ee);

    return CABINET_OK;
}

int EventPoll::processEvent() {
    //监听套接字的事件来到
    //1. 获取连接套接字
    //2. 创建client
    //3. 将client监听可读加入eventpoll

    //client可读
    //1. 接收
    //2. 解析命令
    //3. client解析完成一个命令时, 执行命令
    //
    //notice: client在任意步骤都可能失败, 若失败, 清理client的痕迹
    
    //client可写
    //1. 发送client数据
}

EventPoll::~EventPoll() {
    close(this->eventPollFd);
    //delete all client
}
