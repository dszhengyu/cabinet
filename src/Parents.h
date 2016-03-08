#ifndef PARENTS_H
#define PARENTS_H

#include "ClusterClient.h"
#include <deque>
class ClusterClient;
using std::deque;

class Parents
{
public:
    Parents();
    int addParents();
    int deleteParents();
private:
    deque<ClusterClient *> parentsQueue;

};
#endif

