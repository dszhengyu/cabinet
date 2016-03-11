#include "Server.h"
#include "Log.h"

int main () 
{
    Cabinet *server = new Server();
    server->initConfig();
    server->init();
    server->onFire();

    return 0;
}
