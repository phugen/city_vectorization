#ifndef AUXILIARY_HPP
#define AUXILIARY_HPP

#include <vector>

#include "include/opencvincludes.hpp"
#include "include/connectedcomponent.hpp"

cv::Vec2i pointToVec (cv::Point p);
std::vector<cv::Vec2i> pointToVec (std::vector<cv::Point> pl);

void getBlackLayer(cv::Vec3b thresholds, cv::Mat input, cv::Mat* output);

std::vector<cv::Vec2i> eightConnectedBlackNeighbors(cv::Vec2i pixel, cv::Mat* image);
std::vector<cv::Vec2i> getBlackComponentPixels (cv::Vec2i pixel, cv::Mat* image);
std::vector<cv::Vec2i> getNearestCorners(std::vector<cv::Vec2i> corners, cv::Vec2i pixel, cv::Mat* image, cv::Mat* reconstructed);
void clusterCells (int totalNumberCells, float rhoStep, int numRho, cv::Vec2f primaryCell, std::vector<cv::Vec2f>* clusterLines);
void eraseComponentPixels (ConnectedComponent comp, cv::Mat* image);

bool isValidCoord (cv::Vec2i* check);
void mapHoughToImage (int rows, int cols, float theta, float rho, int numAngle, int numRho, int* accumulator);

void drawLines (std::vector<cv::Vec2f> lines, cv::Mat* image, cv::Scalar color);

double distanceBetweenPoints (cv::Vec2i a, cv::Vec2i b);
double distanceFromCartesianLine(cv::Vec2i point, std::pair<cv::Vec2i, cv::Vec2i> linePoints);
double distanceFromPolarLine (cv::Vec2f point, cv::Vec2f polarLine);
bool pointOnPolarLine (cv::Vec2f point, cv::Vec2f polarLine, double tolerance);

double localAvgHeight (std::vector<ConnectedComponent> cluster, int listPos);



#endif // AUXILIARY_HPP

