/**
  * Contains code related to heuristics that identify text.
  *
  * Author: Philipp Hugenroth
  */

#include "include/stringheuristics.hpp"

// Calculates the local average height from the four neighbors
// of the given component (if possible).
double localAvgHeight (std::vector<ConnectedComponent> cluster, int listPos)
{
    int startPos, endPos;

    // account for comps near the start or end of the cluster list
    listPos - 2 < 0 ? startPos = 0 : startPos = listPos - 2;
    listPos + 2 >= cluster.size() ? endPos = cluster.size() - 1 : endPos = listPos + 2;

    double localAvg = 0;

    // calculate average height
    for(int i = startPos; i <= endPos; i++)
        localAvg += cluster.at(i).mbr_max[0] - cluster.at(i).mbr_min[0];


    return (localAvg /= endPos - startPos);
}
