#ifndef PARENTS_H
#define PARENTS_H

#include "Cluster.h"
#include "ClusterClient.h"
#include <deque>
#include <map>
class Cluster;
class ClusterClient;
using std::deque;
using std::map;

class Parents
{
public:
    typedef deque<ClusterClient *>::iterator queueIter;
    Parents(Cluster *cluster);
    int addParents(ClusterClient *parents);
    int deleteParents(ClusterClient *parents);
    int setDealingIndex(long dealingIndex, ClusterClient *parent);
    ClusterClient *getParentsByDealingIndex(long dealingIndex);
    int shutDown();
private:
    queueIter findParentsInQueue(ClusterClient *);
    deque<ClusterClient *> parentsQueue;
    map<long, ClusterClient *> dealingIndexParentsMap;
    Cluster *cluster;

};
#endif

