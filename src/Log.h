#ifndef LOG_H
#define LOG_H

#include <cstdarg>

static const bool printDebug = true;

class Log
{
public:
    static void fatal(const char *format, ...);
    static void warning(const char *format, ...);
    static void notice(const char *format, ...);
    static void debug(const char *format, ...);

private:
    static void log(const char *logFileName, const char *logHeader, const char *format, va_list args);
    constexpr const static char *wLogFileName = "cabinet.warning.log";
    constexpr const static char *nLogFileName = "cabinet.notice.log";
    const static int BUF_MAX_SIZE = 100;
};

#endif
