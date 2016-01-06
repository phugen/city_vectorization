#ifndef COLLINEARGROUPING_HPP
#define COLLINEARGROUPING_HPP

#include <vector>

#include "opencvincludes.hpp"
#include "connectedcomponent.hpp"

void collinearGrouping (cv::Mat input, std::vector<ConnectedComponent>* comps);

#endif // COLLINEARGROUPING_HPP

