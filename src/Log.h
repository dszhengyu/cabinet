#ifndef LOG_H
#define LOG_H


class Log
{
public:
    static void warning(const char *message);
    static void notice(const char *message);
private:
    constexpr const static char *wLogFileName = "cabinet.warning.log";
    constexpr const static char *nLogFileName = "cabinet.notice.log";
};

#endif
