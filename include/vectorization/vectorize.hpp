#pragma once

#include "include/opencvincludes.hpp"
#include "include/cairo/cairo.h"
#include "include/vectorization/vectorline.hpp"
#include <vector>
#include <map>
#include <set>

struct vec2i_compare;

void vectorizeImage (cv::Mat *blacklayer, cv::Mat *original_image, std::string filename, double epsilon);
std::vector<std::vector<pixel*>> refineVectors(cv::Mat *image, std::map<pixel*, vectorLine*>* nodeToLine, std::vector<pixel *> *pixels, double epsilon);
void vectorsToFile (cv::Mat *image, std::vector<std::vector<pixel*>> paths, std::string filename);
