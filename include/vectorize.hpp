#pragma once

#include "include/opencvincludes.hpp"
#include "include/cairo/cairo.h"
#include <vector>

void vectorizeImage (cv::Mat* image, std::string filename, int maxDist);
void refineVectors (std::vector<cv::Vec4f>* lines, int maxDist, cairo_t *cr);
