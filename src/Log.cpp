#include "Log.h"
#include "Util.h"
#include <fstream>
#include <cstdarg>
#include <cstdio>

using std::ofstream;

void Log::fatal(const char *format, ...) {
    char message[Log::BUF_MAX_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, Log::BUF_MAX_SIZE, format, args);
    ofstream wLogFile(Log::wLogFileName, ofstream::app);
    wLogFile << "Fatal @ " << Util::getCurrentTime() << message << '\n';
    wLogFile.flush();
    wLogFile.close();
}

void Log::warning(const char *format, ...) {
    char message[Log::BUF_MAX_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, Log::BUF_MAX_SIZE, format, args);
    ofstream wLogFile(Log::wLogFileName, ofstream::app);
    wLogFile << "Warning @ " << Util::getCurrentTime() << message << '\n';
    wLogFile.flush();
    wLogFile.close();
}

void Log::notice(const char *format, ...) {
    char message[Log::BUF_MAX_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, Log::BUF_MAX_SIZE, format, args);
    ofstream nLogFile(Log::nLogFileName, ofstream::app);
    nLogFile << "Notice @ " << Util::getCurrentTime() << message << '\n';
    nLogFile.flush();
    nLogFile.close();
}

