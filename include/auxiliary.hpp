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
void eraseConnectedPixels(cv::Vec2i seed, cv::Mat* image);

double getMBRArea(ConnectedComponent comp);
bool isValidCoord (cv::Vec2i* check);

void drawLines (std::vector<cv::Vec2f> lines, cv::Mat* image, cv::Scalar color);

double distanceBetweenPoints (cv::Vec2f a, cv::Vec2f b);
double distanceFromCartesianLine(cv::Vec2f point, std::pair<cv::Vec2f, cv::Vec2f> linePoints, cv::Mat* image);
double distanceFromCartesianSegment(cv::Vec2f point, std::pair<cv::Vec2f, cv::Vec2f> linePoints);
double distanceFromPolarLine (cv::Vec2f point, cv::Vec2f polarLine, cv::Mat* image);
bool pointOnPolarLine (cv::Vec2f point, cv::Vec2f polarLine, double tolerance, cv::Mat* image);

double localAvgHeight (std::vector<ConnectedComponent> cluster, int listPos);
int localAreaDiff (std::vector<ConnectedComponent> cluster, int listPos, bool rev);



#endif // AUXILIARY_HPP

