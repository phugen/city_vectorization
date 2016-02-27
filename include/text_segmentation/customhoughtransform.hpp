#pragma once

#include "include/opencvincludes.hpp"
#include "include/vec2icompare.hpp"

#include <vector>
#include <map>



// Mode defines for Hough extraction method
#define THRESH_GT 0
#define THRESH_EQ 1


// a struct describing a line in Hough space.
struct LinePolar
{
    float rho; // the length of the origin vector.
    float angle; // the angle of the origin vector.
};


void HoughLinesCustom( const cv::Mat& img, float rho, float theta,
                       double min_theta, double max_theta, int* accum,
                       std::map<cv::Vec2i, std::vector<int>, Vec2iCompare>* contributions);

void HoughLinesExtract (int* accum, int numrho, int numangle, float rho, float theta, float min_theta,
                        int threshold, std::vector<cv::Vec3f> *lines, int mode = THRESH_GT);

void deleteLineContributions (int* accum, cv::Vec2i inputPoint, std::map<cv::Vec2i, std::vector<int>, Vec2iCompare> contributions);

