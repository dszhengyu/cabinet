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
    int eventPollFd = epoll_create(this->EPOLL_SIZE);
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
    this->fileEventOperation(client->getClientFd(), eventType, this->DEL_EVENT);

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

    this->fileEventOperation(client->getClientFd(), eventType, this->DEL_EVENT);
    
    mapToEdit->erase(client->getClientFd());

    return CABINET_OK;
}

int EventPoll::pollListenFd(int listenFd) {
    return this->fileEventOperation(listenFd, READ_EVENT, this->ADD_EVENT);
}

int EventPoll::fileEventOperation(int fd, int eventType, int opType) {
    int op = ((opType == this->ADD_EVENT) ? EPOLL_CTL_ADD : EPOLL_CTL_DEL);
    int pollEvent = ((eventType == READ_EVENT) ? EPOLLIN : EPOLLOUT);
    struct epoll_event ee;
    ee.events = pollEvent;
    ee.data.fd = fd;
    epoll_ctl(this->eventPollFd, op, fd, &ee);

    return CABINET_OK;
}


int EventPoll::processEvent() {
    while (1) {
        struct epoll_event events[this->EPOLL_SIZE];
        int eventNumber = epoll_wait(this->eventPollFd, events, this->EPOLL_SIZE, -1);
        if (eventNumber == 0) {
            continue;
        }

        for (int processingNumber = 0; processingNumber < eventNumber; ++eventNumber) {
            int eventFd = events[processingNumber].data.fd;
            auto eventType = events[processingNumber].events;
            //监听套接字的事件来到
            //1. 获取连接套接字
            //2. 创建client
            //3. 将client监听可读加入eventpoll
            if (eventFd == this->server->getListenFd() && eventType & EPOLLIN) {
                int connectFd = 0;
                if ((connectFd = this->server->getConnectFd()) == CABINET_ERR) {
                    Log::warning("get connect fd error");
                    continue;
                }
                Client *newClient = this->server->createClient(connectFd);
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
                if (this->readFileEventMap.find(eventFd) == this->readFileEventMap.end()) {
                    Log::warning("Client lost in read map, maybe deleted before");
                    continue;
                }
                Client *processingClient = readFileEventMap[eventFd];
                if (processingClient->fillInputBuf() == CABINET_ERR) {
                    Log::warning("client client_id[%d] fill input buf error, close it", processingClient->getClientId());
                    this->deleteClient(processingClient);
                    continue;
                }

                if (processingClient->resolveInputBuf() == CABINET_ERR) {
                    Log::warning("client client_id[%d] resolve input buf error, close it", processingClient->getClientId());
                    this->deleteClient(processingClient);
                    continue;
                }

                if (processingClient->isReadyToExecute() == true) {
                    if (processingClient->executeCommand() == CABINET_ERR) {
                        Log::warning("client client_id[%d] execute command error, close it", processingClient->getClientId());
                        this->deleteClient(processingClient);
                        continue;
                    }
                }
                continue;
            }
            
            //client可写
            //1. 发送client数据
            if (eventType & EPOLLOUT) {
                if (this->writeFileEventMap.find(eventFd) == this->writeFileEventMap.end()) {
                    Log::warning("Client lost in read map, maybe deleted before");
                    continue;
                }
                Client *processingClient = writeFileEventMap[eventFd];
                if (processingClient->sendReply() == CABINET_ERR) {
                    Log::warning("client client_id[%d] send reply error, close it", processingClient->getClientId());
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
