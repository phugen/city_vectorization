#ifndef CUSTOMHOUGHTRANSFORM_HPP
#define CUSTOMHOUGHTRANSFORM_HPP

#include "opencvincludes.hpp"
#include <vector>

// Mode defines for Hough extraction method
#define THRESH_GT 0
#define THRESH_EQ 1


// a struct describing a line in Hough space.
struct LinePolar
{
    float rho; // the length of the origin vector.
    float angle; // the angle of the origin vector.
};

void HoughLinesCustom ( const cv::Mat& img, float rho, float theta,
                        double min_theta, double max_theta, int* accumulator);

void HoughLinesExtract (int* accum, int numrho, int numangle, float rho, float theta, float min_theta, int threshold,
                        std::vector<cv::Vec2f>* lines, std::vector<cv::Vec2i>* contributions,
                        int mode = THRESH_GT);

#endif // CUSTOMHOUGHTRANSFORM_HPP

