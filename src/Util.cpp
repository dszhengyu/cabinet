#include "Util.h"
#include "Log.h"
#include "Const.h"
#include <fcntl.h>
#include <stdlib.h>
#include <ctime>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/time.h>

string Util::getCurrentTime() {
    time_t timer = 0;
    time(&timer);
    string timeStr(ctime(&timer));
    timeStr.replace(timeStr.end() - 1, timeStr.end(), ": ");
    return timeStr;
}

long Util::getCurrentTimeInMs() {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == -1) {
        return CABINET_ERR;
    }
    long timeInMs = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return timeInMs;
}

int Util::setNonBlock(int fd) {
    int flags;

    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        return CABINET_ERR;
    }   

    flags |= O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) == -1) {
        return CABINET_ERR;
    }   
    return CABINET_OK;
}

int Util::listenTcp(int port) {
    int listenfd;
    struct sockaddr_in serverAddr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        logFatal("create socket fail!");
        return CABINET_ERR;
    }   

    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        logFatal("bind error!");
        return CABINET_ERR;
    }   

    if (listen(listenfd, 5)< 0) {
        logFatal("listen port error!");
        return CABINET_ERR;
    }
    
    if (Util::setNonBlock(listenfd) == CABINET_ERR) {
        logFatal("set listen fd non-bolck error!");
        return CABINET_ERR;
    }

    logNotice("listening on port %d", port);
    return listenfd;
}

int Util::acceptTcp(const int listenfd, string &strIP, int &port) {
    //获取tcp/ipv4连接
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int connectFd;
    if ((connectFd = accept(listenfd, (struct sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
        if (errno != EWOULDBLOCK) {
            logWarning("accept connect error");
        }
        return CABINET_ERR;
    }

    if (Util::setNonBlock(connectFd) == CABINET_ERR) {
        logWarning("set connect fd non-bolck error!");
        close(connectFd);
        return CABINET_ERR;
    }

    //获取client连接信息
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ip, INET_ADDRSTRLEN);
    strIP = ip;
    port = ntohs(clientAddr.sin_port);

    return connectFd;
}

int Util::connectTcp(const char *ip, int port) {
    logNotice("connect tcp, IP[%s], port[%d]", ip, port);
    int connectFd;
    struct sockaddr_in clientAddr;
    
    if ((connectFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        logWarning("Create Socket Error");
        return CABINET_ERR;
    }

    bzero(&clientAddr, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &clientAddr.sin_addr) <= 0) {
        logWarning("Create Client Address Error");
        return CABINET_ERR;
    }
    
    if (connect(connectFd, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) < 0) {
        logWarning("Connect Server Error, IP[%s], port[%d]", ip, port);
        return CABINET_ERR;
    }
    logNotice("connect tcp success, IP[%s], port[%d], fd[%d]", ip, port, connectFd);
    return connectFd;
}

int Util::daemonize() {
    int pid1 = -1;
    if ((pid1 = fork()) < 0) {
        logWarning("fork first time error");
        return CABINET_ERR;
    }
    else if (pid1 > 0) {
        logDebug("first parent exit");
        exit(0);
    }
    setsid();

    int pid2 = -1;
    if ((pid2 = fork()) < 0) {
        logWarning("fork second time error");
        return CABINET_ERR;
    }
    else if (pid2 > 0) {
        logDebug("second parent exit");
        exit(0);
    }

    int fd;
    if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > STDERR_FILENO) {
            close(fd);
        }
    }
    logNotice("daemonize done. pid[%d]", getpid());
    return CABINET_OK;
}
