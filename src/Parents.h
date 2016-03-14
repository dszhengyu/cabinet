#ifndef PARENTS_H
#define PARENTS_H

#include "ClusterClient.h"
#include <deque>
#include <map>
class ClusterClient;
using std::deque;
using std::map;

class Parents
{
public:
    typedef deque<ClusterClient *>::iterator queueIter;
    Parents();
    int addParents(ClusterClient *parents);
    int deleteParents(ClusterClient *parents);
private:
    queueIter findParentsInQueue(ClusterClient *);
    deque<ClusterClient *> parentsQueue;

};
#endif

