#pragma once

#include <vector>

#include "opencvincludes.hpp"
#include "connectedcomponent.hpp"

struct compareByLineDistance;
void collinearGrouping (cv::Mat input, cv::Mat *output, std::vector<ConnectedComponent>* comps);
