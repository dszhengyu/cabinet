#include "Server.h"

int main () 
{
    Server server;
    server.initConfig();
    server.init();
    server.onFire();

    return 0;
}
