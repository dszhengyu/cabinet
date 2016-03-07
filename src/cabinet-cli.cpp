#include "CabinetCli.h"
#include "Const.h"
#include "Configuration.h"
#include <string>
#include <cstdlib>
#include <signal.h>

CabinetCli *cabinetCli = nullptr;
void *processContinue(int sigo);

int main(int argc, char *argv[])
{
    Configuration conf;
    if (conf.loadConfiguration() == CABINET_ERR) {
        logFatal("load conf error, exit");
        exit(1);
    }
    const char *defaultServerIP = conf["CLI_SERVER_IP"].c_str();
    const int defaultServerPort = std::stoi(conf["CLI_SERVER_PORT"]);

    if (argc == 1) {
        cabinetCli = new CabinetCli(defaultServerIP, defaultServerPort);
    }
    else if (argc == 2) {
        cabinetCli = new CabinetCli(defaultServerIP, atoi(argv[1]));
    }
    else if (argc == 3) {
        cabinetCli = new CabinetCli(argv[1], atoi(argv[2]));
    }

    if (cabinetCli->connectServer() == CABINET_ERR) {
        exit(1);
    }

    if (signal(SIGCONT, (__sighandler_t)processContinue) == SIG_ERR) {
        exit(1);
    }

    while (1) {
        if (cabinetCli->readClientInput() == CABINET_ERR) {
            continue;
        }
        if (cabinetCli->formatClientInput() == CABINET_ERR) {
            continue;
        }
        if (cabinetCli->sendClientInput() == CABINET_ERR) {
            continue;
        }
        if (cabinetCli->receiveServerOutput() == CABINET_ERR) {
            continue;
        }   
        if (cabinetCli->displayServerOutput() == CABINET_ERR) {
            continue;
        }
    }

    return 0;
}

void *processContinue(int signo) {
    cabinetCli->printPrompt();
    return NULL;
}
