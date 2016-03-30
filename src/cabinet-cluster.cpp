#include "Cluster.h"

int main()
{
    Cluster *cluster = new Cluster;
    cluster->initConfig();
    cluster->init();
    cluster->onFire();
    return 0;
}
