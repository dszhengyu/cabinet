#include "Log.h"
#include <fstream>

using std::ofstream;

void Log::warning(const char *message) {
   ofstream wLogFile(Log::wLogFileName, ofstream::app);
   wLogFile << "Warning: " << message;
   wLogFile.flush();
   wLogFile.close();
}

void Log::notice(const char *message) {
   ofstream nLogFile(Log::nLogFileName, ofstream::app);
   nLogFile << "Notice: " << message;
   nLogFile.flush();
   nLogFile.close();
}
