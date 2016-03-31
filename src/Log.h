#ifndef LOG_H
#define LOG_H

#include <cstdarg>

static const bool printDebug = true;

#define logFatal(format, arg...) Log::fatal(__FILE__, __LINE__, format, ##arg)
#define logWarning(format, arg...) Log::warning(__FILE__, __LINE__, format, ##arg)
#define logNotice(format, arg...) Log::notice(__FILE__, __LINE__, format, ##arg)
#define logDebug(format, arg...) Log::debug(__FILE__, __LINE__, format, ##arg)

class Log
{
public:
    static void fatal(const char *fileName, const int line, const char *format, ...);
    static void warning(const char *fileName, const int line, const char *format, ...);
    static void notice(const char *fileName, const int line, const char *format, ...);
    static void debug(const char *fileName, const int line, const char *format, ...);

private:
    static void log(const char *logFileName, const char *logHeader, const char *fileName, int line, const char *format, va_list args);
    //constexpr const static char *wLogFileName = "cabinet.warning.log";
    //constexpr const static char *nLogFileName = "cabinet.notice.log";
    constexpr const static char *wLogFileName = "cabinet.log";
    constexpr const static char *nLogFileName = "cabinet.log";
    const static int BUF_MAX_SIZE = 1024 * 1000;
};

#endif
