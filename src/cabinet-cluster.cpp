#include "Cluster.h"

int main()
{
    Cluster *cluster = new Cluster;
    cluster->initConfig();
    cluster->init();
    return 0;
}
