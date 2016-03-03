#pragma once

#include "include/opencvincludes.hpp"
#include<vector>

void getBlackLayer(cv::Vec3b thresholds, cv::Mat input, cv::Mat* output);
void negateBlackLayer(int threshold,cv::Mat* input, cv::Mat* output);
void getWhiteLayer(int threshold, cv::Mat* input, cv::Mat* output);

void extendLine(cv::Mat* src, cv::Vec4i* srcline, float eps);
void extendAllLines(cv::Mat* src, std::vector<cv::Vec4i>* lineList, float eps);
void drawLineCollection(cv::Mat* target, std::vector<cv::Vec4i> lineList,int thickness, cv::Scalar color);
void appendLineCollection(std::vector<cv::Vec4i>* appendTo, std::vector<cv::Vec4i>* appendFrom, bool shouldClear = false);
bool edgeAlignment(std::vector<cv::Vec4i>* lineCollection, float gapTH, int rows, int cols);
int getPointOfIntersection(cv::Vec4i L1, cv::Vec4i L2, cv::Point* output);
void customThinning(cv::Mat* target, int numIterations, int cannyParam);
void removeLineFromBlackLayer(cv::Mat* target, std::vector<cv::Vec4i> lineCollection);
void pointsToLines(cv::Mat* target,cv::Mat* drawTarget, std::vector<cv::Vec4i>* lineCollection);
void getFinalLines(cv::Mat* blacklayer, std::vector<cv::Vec4i>* dirLineCollection, char* source_window, bool debug = (false));
