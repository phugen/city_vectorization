#pragma once

#include <vector>

#include "opencvincludes.hpp"
#include "connectedcomponent.hpp"

struct compareByLineDistance;
void collinearGrouping (cv::Mat input, std::vector<ConnectedComponent>* comps);
