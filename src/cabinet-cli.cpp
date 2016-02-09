#include "CabinetCli.h"
#include "Const.h"

int main(int argc, char *argv[])
{
    CabinetCli cabinetCli;
    if (cabinetCli.connectServer() == CABINET_ERR) {
        exit(1);
    }

    while (1) {
        if (cabinetCli.readClientInput() == CABINET_ERR) {
            continue;
        }
        if (cabinetCli.formatClientInput() == CABINET_ERR) {
            continue;
        }
        if (cabinetCli.sendClientInput() == CABINET_ERR) {
            continue;
        }
        if (cabinetCli.receiveServerOutput() == CABINET_ERR) {
            continue;
        }   
        if (cabinetCli.displayServerOutput() == CABINET_ERR) {
            continue;
        }
    }

    return 0;
}
