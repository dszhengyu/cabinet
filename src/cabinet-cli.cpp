#include "CabinetCli.h"
#include "Const.h"
#include <cstdlib>

int main(int argc, char *argv[])
{
    CabinetCli *cabinetCli = nullptr;
    if (argc == 1) {
        cabinetCli = new CabinetCli();
    }
    else if (argc == 2) {
        cabinetCli = new CabinetCli(atoi(argv[1]));
    }
    else if (argc == 3) {
        cabinetCli = new CabinetCli(argv[1], atoi(argv[2]));
    }

    if (cabinetCli->connectServer() == CABINET_ERR) {
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
