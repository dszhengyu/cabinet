#include "Log.h"
#include "Util.h"
#include <fstream>
#include <cstdio>
#include <iostream>

using std::ofstream;

void Log::fatal(const char *fileName, const int line, const char *format, ...) {
    va_list args;
    va_start(args, format);
    Log::log(Log::wLogFileName, "Fatal   @ ", fileName, line, format, args);
}

void Log::warning(const char *fileName, const int line, const char *format, ...) {
    va_list args;
    va_start(args, format);
    Log::log(Log::wLogFileName, "Warning @ ", fileName, line, format, args);
}

void Log::notice(const char *fileName, const int line, const char *format, ...) {
    va_list args;
    va_start(args, format);
    Log::log(Log::nLogFileName, "Notice  @ ", fileName, line, format, args);
}

void Log::debug(const char *fileName, const int line, const char *format, ...) {
    if (!printDebug) {
        return;
    }   
    va_list args;
    va_start(args, format);
    Log::log(Log::nLogFileName, "Debug   @ ", fileName, line, format, args);
}

void Log::log(const char *logFileName, const char *logHeader, const char *fileName, int line, const char *format, va_list args) {
    char message[Log::BUF_MAX_SIZE];
    vsnprintf(message, Log::BUF_MAX_SIZE, format, args);
    ofstream logFile(logFileName, ofstream::app);
    logFile << logHeader << Util::getCurrentTime() << fileName << ":" << line << " : " << message << '\n';
    logFile.flush();
    logFile.close();
}

