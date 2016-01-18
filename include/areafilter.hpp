#ifndef AREAFILTER_HPP
#define AREAFILTER_HPP

#include <vector>

#include "opencvincludes.hpp"
#include "connectedcomponent.hpp"

void areaFilter(std::vector<ConnectedComponent>* components, int ratio);
void clusterCompAreaFilter(std::vector<ConnectedComponent>* cluster, int ratio);

#endif // AREAFILTER_HPP

