#pragma once

#include "include/opencvincludes.hpp"
#include "include/cairo/cairo.h"
#include "include/vectorline.hpp"
#include <vector>
#include <map>
#include <set>

struct vec2i_compare;

uint8_t encodeNeighbors (cv::Mat* image, cv::Vec2i curPixel);
void addToTable (std::vector<int>* neighborhoods, int *ruleTable, int rule);
void initRuleTable(int* ruleTable);

void closeMultiple (std::set<vectorLine*>* lines, std::map<cv::Vec2i, vectorLine*, vec2i_compare>* pxToLine,
                      std::map<cv::Vec2i, vectorLine*, vec2i_compare>* nodeToLine, cv::Vec2i cur, cv::Vec2i end1, cv::Vec2i end2);

void connectLines(std::set<vectorLine*>* lines, std::map<cv::Vec2i, vectorLine*, vec2i_compare>* pxToLine, std::map<cv::Vec2i, vectorLine *, vec2i_compare>* nodeToLine,
                  cv::Vec2i cur, cv::Vec2i conn1, cv::Vec2i conn2);

void extendLine(std::set<vectorLine*>* lines, std::map<cv::Vec2i, vectorLine*, vec2i_compare>* pxToLine, std::map<cv::Vec2i, vectorLine *, vec2i_compare>* nodeToLine,
                cv::Vec2i cur, cv::Vec2i otherPx);

void closeSingle(std::set<vectorLine*>* lines, std::map<cv::Vec2i, vectorLine*, vec2i_compare>* pxToLine, std::map<cv::Vec2i, vectorLine*, vec2i_compare>* nodeToLine,
                 cv::Vec2i cur, cv::Vec2i otherPx);

vectorLine* makeNode(std::set<vectorLine*>* lines, std::map<cv::Vec2i, vectorLine*, vec2i_compare>* pxToLine,
                std::map<cv::Vec2i, vectorLine*, vec2i_compare>* nodeToLine, cv::Vec2i cur);

void applyRule(cv::Vec2i cur, uint8_t nBits, int* ruleTable, std::map<cv::Vec2i, vectorLine*, vec2i_compare>* pxToLine, std::map<cv::Vec2i, vectorLine *, vec2i_compare> *nodeToLine,
               std::set<vectorLine*>* lines);

bool isNode(cv::Vec2i cur, std::map<cv::Vec2i, vectorLine*, vec2i_compare>* pxToLine);

void vectorizeImage (cv::Mat* image, std::string filename);
void refineVectors (std::set<vectorLine*>* lines);
void vectorsToFile (cv::Mat *image, std::set<vectorLine *> lines, std::string filename);
