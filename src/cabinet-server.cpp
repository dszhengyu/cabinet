#include "Server.h"

int main () 
{
    Server server;
    server.initConfig();
    server.init();
    server.createClient(4);

    return 0;
}
