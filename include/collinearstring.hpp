#ifndef COLLINEARSTRING_HPP
#define COLLINEARSTRING_HPP

#include <vector>
#include "include/connectedcomponent.hpp"

class CollinearString
{
public:
    std::vector<ConnectedComponent> comps; // all components that are part of this string

    CollinearString(std::vector<ConnectedComponent> cluster);

    // finds average height in a 5-neighborhood
    double localAvgHeight (std::vector<ConnectedComponent> cluster, int listPos);

    // finds smallest distance between current and next component MBR
    double edgeToEdgeDistance (std::vector<ConnectedComponent> cluster, int listPos);

private:
    // delete all components from this string that do not fit the
    // string heuristics
    void refine();
};

#endif // COLLINEARSTRING_HPP

