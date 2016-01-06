#ifndef AUXILIARY_HPP
#define AUXILIARY_HPP

#include <vector>
#include "include/opencvincludes.hpp"

bool isBlack (cv::Vec3b check);
bool isBlack (uchar check);
void getBlackLayer(cv::Vec3b thresholds, cv::Mat input, cv::Mat* output);
bool isValidCoord (cv::Vec2i* check);
void drawLines (std::vector<cv::Vec2f> lines, cv::Mat* image, cv::Scalar color);
bool pointOnPolarLine (cv::Vec2f point, cv::Vec2f polarLine, double tolerance);
void clusterCells (int totalNumberCells, float rhoStep, int numRho, cv::Vec2f primaryCell, vector<cv::Vec2f>* clusterLines);

#endif // AUXILIARY_HPP

