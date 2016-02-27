#pragma once

#include "include/opencvincludes.hpp"
#include "include/cairo/cairo.h"
#include "include/vectorization/vectorline.hpp"
#include <vector>
#include <map>
#include <set>

struct vec2i_compare;

void vectorizeImage (cv::Mat* image, std::string filename);
void refineVectors (std::map<pixel*, vectorLine*>* nodeToLine);
void vectorsToFile (cv::Mat *image, std::set<vectorLine*> refinedLines, std::string filename);
