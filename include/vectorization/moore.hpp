#pragma once

#include "include/opencvincludes.hpp"
#include "include/vectorization/vectorline.hpp"
#include <vector>
#include <set>

uint8_t encodeNeighbors (cv::Mat* image, cv::Vec2i curPixel);
void addToTable (std::vector<int>* neighborhoods, int *ruleTable, int rule);
void initRuleTable(int* ruleTable);
void initPixels(std::vector<vectorLine*>* pixels, cv::Mat* image);

void applyRule(cv::Mat* image, pixel* cur, uint8_t nBits, int* ruleTable,
               std::set<vectorLine*>* lines, std::vector<pixel *> *pixels);

std::map<pixel*, vectorLine*> mooreVector(cv::Mat image, std::vector<pixel*>* pixels, pixel* dummy);
