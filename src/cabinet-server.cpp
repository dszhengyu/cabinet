#include "Server.h"

int main () 
{
    Server server;
    server.initConfig();
    server.init();
    server.createClient();

    return 0;
}
