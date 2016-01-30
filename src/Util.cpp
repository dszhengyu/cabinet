#include "Util.h"
#include <ctime>

#include <iostream>
string Util::getCurrentTime() {
    time_t timer = 0;
    time(&timer);
    string timeStr(ctime(&timer));
    timeStr.replace(timeStr.end() - 1, timeStr.end(), ": ");
    return timeStr;
}
