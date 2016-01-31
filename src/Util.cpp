#include "Util.h"
#include <fcntl.h>
#include <ctime>

string Util::getCurrentTime() {
    time_t timer = 0;
    time(&timer);
    string timeStr(ctime(&timer));
    timeStr.replace(timeStr.end() - 1, timeStr.end(), ": ");
    return timeStr;
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
