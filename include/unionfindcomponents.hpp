#pragma once

#include <vector>

#include "opencvincludes.hpp"
#include "include/connectedcomponent.hpp"

void unionFindComponents(cv::Mat* input, std::vector<ConnectedComponent>* components, int minPx);
