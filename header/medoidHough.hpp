#ifndef MEDOIDHOUGH_HPP
#define MEDOIDHOUGH_HPP

#include "opencvincludes.hpp"

void medoidHough (cv::Vec2f center, int* accumulator, int rho, int theta, std::vector<cv::Vec2f> output);

#endif // MEDOIDHOUGH_HPP

