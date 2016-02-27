#pragma once

#include "include/opencvincludes.hpp"
#include "include/cairo/cairo.h"
#include "include/vectorization/vectorline.hpp"
#include <vector>
#include <map>
#include <set>

struct vec2i_compare;

void vectorizeImage (cv::Mat* image, std::string filename);
std::vector<std::vector<vectorLine *> > refineVectors(cv::Mat *image, std::map<pixel*, vectorLine*>* nodeToLine, std::vector<pixel *> *pixels);
void vectorsToFile (cv::Mat *image, std::set<vectorLine*> refinedLines, std::string filename);
