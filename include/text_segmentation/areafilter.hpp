#pragma once

#include <vector>

#include "include/opencvincludes.hpp"
#include "connectedcomponent.hpp"


void areaFilter(std::vector<ConnectedComponent>* components, int ratio);
void clusterCompAreaFilter(std::vector<ConnectedComponent>* cluster, double maxError);

