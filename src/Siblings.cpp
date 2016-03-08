#include "Siblings.h"
#include "Const.h"

Siblings::Siblings()
{
}

int Siblings::recognizeSiblings(const Configuration &conf) {
    return CABINET_OK;
}

int Siblings::addSiblings(ClusterClient *sibling) {
    return CABINET_OK;
}

int Siblings::deleteSiblings(ClusterClient *sibling) {
    return CABINET_OK;
}
