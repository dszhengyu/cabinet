#include "EventPoll.h"
#include "Const.h"
#include <sys/epoll.h>
#include <unistd.h>

EventPoll::EventPoll(Cabinet *cabinet):
    cabinet(cabinet),
    readFileEventMap(),
    writeFileEventMap(),
    listenFdSet()
{

}

int EventPoll::initEventPoll() {
    int eventPollFd = epoll_create(this->EPOLL_SIZE);
    if (eventPollFd == -1) {
        logFatal("create epoll error");
        return CABINET_ERR;
    }
    this->eventPollFd = eventPollFd;
    return CABINET_OK;
}

/*
 * brief: 添加文件事件, 在epoll中添加, 在map中也需要添加
 */
int EventPoll::createFileEvent(Client *client, int eventType) {
    logDebug("event poll create file event, client client_id[%d], eventType[%d]", client->getClientId(), eventType);
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
    this->fileEventOperation(client->getClientFd(), eventType, this->ADD_EVENT);

    //modify map
    (*mapToEdit)[client->getClientFd()] = client;

    return CABINET_OK;
}

/*
 * brief: remove file event both in epoll and map
 */
int EventPoll::removeFileEvent(Client *client, int eventType) {
    logDebug("event poll remove file event, client client_id[%d], eventType[%d]", client->getClientId(), eventType);
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
        logWarning("file event already deleted, client_id[%d]", client->getClientId());
        return CABINET_OK;
    }

    this->fileEventOperation(client->getClientFd(), eventType, this->DEL_EVENT);
    
    mapToEdit->erase(client->getClientFd());

    return CABINET_OK;
}

int EventPoll::pollListenFd(int listenFd) {
    logDebug("event poll poll listen fd[%d]", listenFd);
    if (this->listenFdSet.find(listenFd) != this->listenFdSet.end()) {
        logWarning("listen fd already added in event poll, listen fd[%d]", listenFd);
        return CABINET_ERR;
    }
    this->listenFdSet.insert(listenFd);
    return this->fileEventOperation(listenFd, READ_EVENT, this->ADD_EVENT);
}

int EventPoll::fileEventOperation(int fd, int eventType, int opType) {
    int op = ((opType == this->ADD_EVENT) ? EPOLL_CTL_ADD : EPOLL_CTL_DEL);
    //int op = ((opType == this->ADD_EVENT) ? EPOLL_CTL_ADD : ((opType == this->DEL_EVENT) ? EPOLL_CTL_DEL : EPOLL_CTL_MOD));
    int pollEvent = ((eventType == READ_EVENT) ? EPOLLIN : EPOLLOUT);
    struct epoll_event ee;
    ee.events = pollEvent;
    ee.data.fd = fd;
    logDebug("epoll_ctl fd[%d], event_type[%d], op_type[%d]", fd, eventType, opType);
    if (epoll_ctl(this->eventPollFd, op, fd, &ee) != 0) {
        logFatal("epoll_ctl error, fd[%d], event_type[%d], op_type[%d], errno[%d]", fd, eventType, opType, errno);
        exit(1);
    }
    return CABINET_OK;
}


int EventPoll::processEvent() {
    while (1) {
        //do cabinet time event
        this->cabinet->cron();
        int timeout = this->cabinet->nextCronTime();

        //do file event
        struct epoll_event events[this->EPOLL_SIZE];
        logDebug("###epoll wait");
        int eventNumber = epoll_wait(this->eventPollFd, events, this->EPOLL_SIZE, timeout);
        logDebug("###event comes~ event_number[%d]", eventNumber);
        if (eventNumber == 0) {
            continue;
        }

        for (int processingNumber = 0; processingNumber < eventNumber; ++processingNumber) {
            int eventFd = events[processingNumber].data.fd;
            auto eventType = events[processingNumber].events;
            //监听套接字的事件来到
            //1. 创建client
            //2. 将client监听可读加入eventpoll
            if ((this->listenFdSet.find(eventFd) != this->listenFdSet.end()) && (eventType & EPOLLIN)) {
                logDebug("event poll listen fd readable");
                Client *newClient = this->cabinet->createClient(eventFd);
                if (newClient == nullptr) {
                    logNotice("create client error");
                    continue;
                }
                this->createFileEvent(newClient, READ_EVENT);
                continue;
            }

            //client可读
            //1. 接收
            //2. 解析命令
            //3. client解析完成一个命令时, 执行命令
            //
            //notice: client在任意步骤都可能失败, 若失败, 清理client的痕迹
            if (eventType & EPOLLIN) {
                logDebug("event poll client fd readable");
                if (this->readFileEventMap.find(eventFd) == this->readFileEventMap.end()) {
                    logWarning("Client lost in read map, maybe deleted before");
                    continue;
                }
                Client *processingClient = readFileEventMap[eventFd];
                if (processingClient->fillReceiveBuf() == CABINET_ERR) {
                    logWarning("client client_id[%d] fill input buf error, maybe client close connection, close it", 
                            processingClient->getClientId());
                    this->deleteClient(processingClient);
                    continue;
                }

                if (processingClient->resolveReceiveBuf() == CABINET_ERR) {
                    logWarning("client client_id[%d] resolve input buf error, close it", processingClient->getClientId());
                    this->deleteClient(processingClient);
                    continue;
                }

                if (processingClient->isReceiveComplete() == true) {
                    if (processingClient->executeCommand() == CABINET_ERR) {
                        logWarning("client client_id[%d] execute command error, close it", processingClient->getClientId());
                        this->deleteClient(processingClient);
                        continue;
                    }
                }
                continue;
            }
            
            //client可写
            //1. 发送client数据
            if (eventType & EPOLLOUT) {
                logDebug("event poll client fd writeable");
                if (this->writeFileEventMap.find(eventFd) == this->writeFileEventMap.end()) {
                    logWarning("Client lost in read map, maybe deleted before");
                    continue;
                }
                Client *processingClient = writeFileEventMap[eventFd];
                if (processingClient->sendReply() == CABINET_ERR) {
                    logWarning("client client_id[%d] send reply error, close it", processingClient->getClientId());
                    this->deleteClient(processingClient);
                    continue;
                }
                continue;
            }
        }
    }
    return CABINET_OK;
}

int EventPoll::deleteClient(Client *client) {
    logWarning("event poll delete client client_id[%d]", client->getClientId());
    this->cabinet->deleteClient(client);
    this->removeFileEvent(client, READ_EVENT);
    this->removeFileEvent(client, WRITE_EVENT);
    delete client;
    return CABINET_OK;
}

EventPoll::~EventPoll() {
    close(this->eventPollFd);

    //delete all client
    //all client must exist in readFileEventMap
    for (auto &mapValue : this->readFileEventMap) {
        Client *deletingClient = mapValue.second;
        this->deleteClient(deletingClient);
    }
}
