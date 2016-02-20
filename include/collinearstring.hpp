#pragma once

#include <vector>
#include "include/connectedcomponent.hpp"
#include "include/collineargroup.hpp"
#include "include/collinearphrase.hpp"

class CollinearString
{
public:

    CollinearString(std::vector<ConnectedComponent> cluster, double avgHeight);
    ~CollinearString();

    std::vector<ConnectedComponent> comps; // all components that are part of this string
    std::vector<CollinearGroup> groups; // word groups in this string
    std::vector<CollinearPhrase> phrases; // phrases in this string

    int groupNo; // actual number of assigned groups
    int phraseNo; // actual number of assigned phrases

    // finds average height in a 5-neighborhood
    double localAvgHeight (std::vector<ConnectedComponent> cluster, int listPos);

    // finds smallest distance between current and next component MBR
    double edgeToEdgeDistance (std::vector<ConnectedComponent> cluster, int listPos);

    // Classifies components in this string.
    void refine();

};

