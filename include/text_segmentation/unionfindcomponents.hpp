#pragma once

#include <vector>

#include "include/opencvincludes.hpp"
#include "include/text_segmentation/connectedcomponent.hpp"

void unionFindComponents(cv::Mat* input, std::vector<ConnectedComponent>* components, int minPx);
