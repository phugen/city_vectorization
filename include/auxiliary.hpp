#ifndef AUXILIARY_HPP
#define AUXILIARY_HPP

#include <vector>

#include "include/opencvincludes.hpp"
#include "include/connectedcomponent.hpp"

void getBlackLayer(cv::Vec3b thresholds, cv::Mat input, cv::Mat* output);
bool isBlack (uchar check);

std::vector<cv::Vec2i> eightConnectedNeighbors(cv::Vec2i pixel, cv::Mat* image);
std::vector<cv::Vec2i> getBlackComponentPixels (cv::Vec2i pixel, cv::Mat* image);
void eraseComponentPixels (ConnectedComponent comp, cv::Mat* image);

bool isValidCoord (cv::Vec2i* check);

void drawLines (std::vector<cv::Vec2f> lines, cv::Mat* image, cv::Scalar color);
bool pointOnPolarLine (cv::Vec2f point, cv::Vec2f polarLine, double tolerance);

void clusterCells (int totalNumberCells, float rhoStep, int numRho, cv::Vec2f primaryCell, std::vector<cv::Vec2f>* clusterLines);

#endif // AUXILIARY_HPP

