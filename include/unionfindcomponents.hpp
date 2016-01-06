#ifndef UNIONFIND_HPP
#define UNIONFIND_HPP

#include <vector>
#include "opencvincludes.hpp"

void unionFindComponents(cv::Mat input, std::vector<ConnectedComponent>* components);

#endif // UNIONFIND_HPP

