#ifndef LOG_H
#define LOG_H


class Log
{
public:
    static void fatal(const char *format, ...);
    static void warning(const char *format, ...);
    static void notice(const char *format, ...);
private:
    constexpr const static char *wLogFileName = "cabinet.warning.log";
    constexpr const static char *nLogFileName = "cabinet.notice.log";
    const static int BUF_MAX_SIZE = 100;
};

#endif
