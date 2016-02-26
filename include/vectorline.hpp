#pragma once

#include "include/opencvincludes.hpp"
#include <vector>

class vectorLine;

/**
  * Defines a pixel that can be either part of a line, or
  * a node (a line endpoint). Intended for use in the
  * L. Moore vectorization algorithm.
  *
  * Author: phugen
  */
class pixel
{
public:
    pixel(cv::Vec2i coord);
    pixel(cv::Vec2i coord, vectorLine* line, bool isNode);
    ~pixel();

    cv::Vec2i coord; // (row, col)
    vectorLine* line;
    bool isNode;

    bool operator < (const pixel* other) const;
};


/**
  *  Describes a 2D line, defined by its two endpoints (nodes).
  *  Intended for usage in the L. Moore vectorization algorithm.
  *
  *  Author: phugen
  */
class vectorLine
{
public:
    vectorLine();
    ~vectorLine();

    pixel* getStart();
    pixel* getEnd();
    void setStart(pixel* pos);
    void setEnd(pixel* pos);

    std::vector<pixel*> getPixels();
    void addPixels (pixel* px);
    void addPixels(std::vector<pixel*>::iterator start, std::vector<pixel*>::iterator end);

    bool operator < (const vectorLine other) const;

private:
    pixel* start;
    pixel* end;
    std::vector<pixel*> pixels; // all pixels that belong to this line (EXCLUDING start and end)
};

