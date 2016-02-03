#ifndef UNIONFINDCOMPONENTS_HPP
#define UNIONFINDCOMPONENTS_HPP

#include <vector>

#include "opencvincludes.hpp"
#include "include/connectedcomponent.hpp"

void unionFindComponents(cv::Mat* input, std::vector<ConnectedComponent>* components, int minPx);

#endif // UNIONFINDCOMPONENTS_HPP

