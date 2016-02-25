/**
  *  Describes a 2D line, defined by its two endpoints (nodes).
  *  Intended for usage in the L. Moore vectorization algorithm.
  *
  *  Author: phugen
  */

#pragma once

#include "include/opencvincludes.hpp"
#include <vector>


class vectorLine
{
public:
    vectorLine();
    ~vectorLine();

    cv::Vec2i getStart();
    cv::Vec2i getEnd();
    void setStart(cv::Vec2i pos);
    void setEnd(cv::Vec2i pos);

    std::vector<cv::Vec2i> getPixels ();
    void addPixels (cv::Vec2i px);
    void addPixels(std::vector<cv::Vec2i>::iterator start, std::vector<cv::Vec2i>::iterator end);

    bool operator < (const vectorLine other) const;

private:
    cv::Vec2i start;
    cv::Vec2i end;
    std::vector<cv::Vec2i> pixels; // all pixels that belong to this line (EXCLUDING start and end)
};

