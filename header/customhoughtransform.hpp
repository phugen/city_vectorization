#ifndef CUSTOMHOUGHTRANSFORM_HPP
#define CUSTOMHOUGHTRANSFORM_HPP

#include "opencvincludes.hpp"
#include <vector>

// a struct describing a line in Hough space.
struct LinePolar
{
    float rho; // the length of the origin vector.
    float angle; // the angle of the origin vector.
};

void HoughLinesCustom( const cv::Mat& img, float rho, float theta,
                        int threshold, std::vector<cv::Vec2f>& lines, int linesMax,
                        double min_theta, double max_theta, int* _accum);

#endif // CUSTOMHOUGHTRANSFORM_HPP

