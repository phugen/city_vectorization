#ifndef COLLINEARGROUP_HPP
#define COLLINEARGROUP_HPP

#include "include/connectedcomponent.hpp"

class CollinearGroup
{
public:
    CollinearGroup();
    ~CollinearGroup();

    int size(); // returns number of components in this group

    char type;  // denotes whether this group is isolated ('i'), a word ('w') or even part of a phrase ('p')
    std::vector<ConnectedComponent> chars; // contains pointers to all components that are part of this group
};

#endif // COLLINEARGROUP_HPP

