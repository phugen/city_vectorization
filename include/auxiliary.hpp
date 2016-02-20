#pragma once

#include <vector>

#include "include/opencvincludes.hpp"
#include "include/connectedcomponent.hpp"

cv::Vec2i pointToVec (cv::Point p);
std::vector<cv::Vec2i> pointToVec (std::vector<cv::Point> pl);

void getBlackLayer(cv::Vec3b thresholds, cv::Mat input, cv::Mat* output);

std::vector<cv::Vec2i> eightConnectedBlackNeighbors(cv::Vec2i pixel, cv::Mat* image);
std::vector<cv::Vec2i> getBlackComponentPixels (cv::Vec2i pixel, cv::Mat* image);
//std::vector<cv::Vec2i> getNearestCorners(std::vector<cv::Vec2i> corners, cv::Vec2i pixel, cv::Mat* image, cv::Mat* reconstructed);
void clusterCells (int totalNumberCells, float rhoStep, int numRho, cv::Vec3f primaryCellPos, std::vector<cv::Vec3f>* lines);
void eraseComponentPixels (ConnectedComponent comp, cv::Mat* image);
void eraseConnectedPixels(cv::Vec2i seed, cv::Mat* image);

double getMBRArea(ConnectedComponent comp);
bool isValidCoord (cv::Vec2i* check);

void drawLines (std::vector<cv::Vec3f> lines, cv::Mat* image, cv::Scalar color);

double slope (double poX0, double poY0, double poX1, double poY1);
double distanceBetweenPoints (cv::Vec2f a, cv::Vec2f b);
double distanceFromCartesianLine(cv::Vec2f point, std::pair<cv::Vec2f, cv::Vec2f> linePoints, cv::Mat* image);
double distanceFromCartesianSegment(cv::Vec2f point, std::pair<cv::Vec2f, cv::Vec2f> linePoints);

double distanceFromPolarLine (cv::Vec2f point, cv::Vec3f polarLine);
bool pointOnPolarLine (cv::Vec2f point, cv::Vec3f polarLine, double tolerance, cv::Mat* image);
bool pointBetweenPolarLines (cv::Vec2f point, cv::Vec3f polar1, cv::Vec3f polar2);
int pointDirectionFromPolarLine (cv::Vec2f point, cv::Vec3f polarLine);
bool areParallelPolar (cv::Vec3f polar1, cv::Vec3f polar2);

double localAvgHeight (std::vector<ConnectedComponent> cluster, int listPos);
int localAreaDiff (std::vector<ConnectedComponent> cluster, int listPos, bool rev);

