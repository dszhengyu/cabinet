#include "Log.h"
#include "Util.h"
#include <fstream>
#include <cstdio>

using std::ofstream;

void Log::fatal(const char *format, ...) {
    va_list args;
    va_start(args, format);
    Log::log(Log::wLogFileName, "Fatal @ ", format, args);
}

void Log::warning(const char *format, ...) {
    va_list args;
    va_start(args, format);
    Log::log(Log::wLogFileName, "Warning @ ", format, args);
}

void Log::notice(const char *format, ...) {
    va_list args;
    va_start(args, format);
    Log::log(Log::nLogFileName, "Notice @ ", format, args);
}

void Log::debug(const char *format, ...) {
    if (!printDebug) {
        return;
    }   
    va_list args;
    va_start(args, format);
    Log::log(Log::nLogFileName, "Debug @ ", format, args);
}

void Log::log(const char *logFileName, const char *logHeader, const char *format, va_list args) {
    char message[Log::BUF_MAX_SIZE];
    vsnprintf(message, Log::BUF_MAX_SIZE, format, args);
    ofstream logFile(logFileName, ofstream::app);
    logFile << logHeader << Util::getCurrentTime() << message << '\n';
    logFile.flush();
    logFile.close();
}

