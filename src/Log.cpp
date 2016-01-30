#include "Log.h"
#include "Util.h"
#include <fstream>

using std::ofstream;

void Log::warning(const char *message) {
   ofstream wLogFile(Log::wLogFileName, ofstream::app);
   wLogFile << "Warning " << Util::getCurrentTime() << message << '\n';
   wLogFile.flush();
   wLogFile.close();
}

void Log::notice(const char *message) {
   ofstream nLogFile(Log::nLogFileName, ofstream::app);
   nLogFile << "Notice " << Util::getCurrentTime() << message << '\n';
   nLogFile.flush();
   nLogFile.close();
}
