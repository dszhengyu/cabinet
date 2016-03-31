#include "Cluster.h"
#include "Log.h"
#include <signal.h>
#include <cstdlib>

Cluster *cluster = nullptr;
void *processShutDown(int sigo);

int main()
{
    if (signal(SIGTERM, (__sighandler_t)processShutDown) == SIG_ERR) {
        logFatal("install shut down function fail");
        exit(1);
    }
    cluster = new Cluster;
    cluster->initConfig();
    cluster->init();
    cluster->onFire();
    return 0;
}

void *processShutDown(int sigo) {
    if (cluster == nullptr) {
        return nullptr;
    }
    logFatal("receive signal to shutdown cabint-cluster");
    cluster->shutDown();
    exit(1);
    return nullptr;
}
