#ifndef COLORCONVERSIONS_HPP
#define COLORCONVERSIONS_HPP

#include "opencvincludes.hpp"

cv::Vec3i RGBtoHSV (cv::Vec3i rgb);
cv::Vec3i HSVtoRGB (cv::Vec3i hsv);
cv::Vec3i intToRGB (cv::Vec2i range, int value);

#endif // COLORCONVERSIONS_HPP

