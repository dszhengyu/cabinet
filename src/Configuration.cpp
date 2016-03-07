#include "Configuration.h"
#include "Const.h"
#include "Log.h"
#include <fstream>
using std::ifstream;
using std::getline;

int Configuration::loadConfiguration() {
    return this->loadConfiguration(Configuration::defaultConfFile);
}

int Configuration::loadConfiguration(const char *confFile) {
    ifstream confFileIn(confFile); 
    if (!confFileIn.good()) {
        logWarning("open conf file fail. filename[%s]", confFile);
        return CABINET_ERR;
    }

    string lineBuf;
    while (confFileIn.good()) {
        getline(confFileIn, lineBuf);
        logDebug("get conf line[%s]", lineBuf.c_str());
        if ((lineBuf.length() == 0) || (lineBuf[0] == '#')) {
            logDebug("line length == 0 or line is comment, skip");
            continue;
        }
        size_t sepPos = -1;
        if ((sepPos = lineBuf.find("=")) == string::npos) {
            logDebug("line miss : , skip");
            continue;
        }
        string key(lineBuf, 0, sepPos);
        logDebug("conf key[%s]", key.c_str());
        string value(lineBuf, sepPos + 1, lineBuf.length());
        logDebug("conf value[%s]", value.c_str());
        configuration[key] = value;
    }

    return CABINET_OK;
}

const string Configuration::operator[](const char *confName) {
    return this->operator[](string(confName));
}

const string Configuration::operator[](const string &confName) {
    if (configuration.find(confName) == configuration.end()) {
        return string("undefine");
    }
    return this->configuration[confName];
}
