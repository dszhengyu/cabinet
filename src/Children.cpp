#include "Children.h"
#include "Const.h"

Children::Children()
{
}

int Children::recognizeChildren(const Configuration &conf) {
    return CABINET_OK;
}

int Children::addChildren(ClusterClient *child) {
    return CABINET_OK;
}

int Children::deleteChildren(ClusterClient *child) {
    return CABINET_OK;
}
