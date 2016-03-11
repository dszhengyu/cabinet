#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <map>
using std::string;
using std::map;

class Configuration
{
public:
    int loadConfiguration();
    int loadConfiguration(const char *confFile);
    const string operator[](const char *confName);
    const string operator[](const string &confName);

private:
    typedef map<string, string> configuration_t;
    configuration_t configuration;
    constexpr const static char * defaultConfFile = "cabinet.conf";
};

#endif
