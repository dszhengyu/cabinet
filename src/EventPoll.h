#ifndef EVENTPOLL_H
#define EVENTPOLL_H

#include "Server.h"
#include "Client.h"
#include <string>
#include <map>
class Server;
class Client;

using std::string;
using std::map;

class EventPoll
{
public :
    typedef map<int, Client *> fileeventmap_t;
    EventPoll(Server *server);
    int initEventPoll();
    int createFileEvent(Client *client, int eventType);
    int removeFileEvent(Client *client, int eventType);
    int processEvent();
    int deleteClient(Client *client);
    int pollListenFd(int listenFd);
    ~EventPoll();

private:
    int fileEventOperation(int fd, int eventType, int opType);
    Server *server;
    fileeventmap_t readFileEventMap;
    fileeventmap_t writeFileEventMap;
    int eventPollFd;
    const int ADD_EVENT = 0;
    const int DEL_EVENT = 1;
    const int EPOLL_SIZE = 1024;
};

#endif
