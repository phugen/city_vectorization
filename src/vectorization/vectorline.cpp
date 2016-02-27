#include "include/vectorization/vectorline.hpp"
#include <iostream>

using namespace std;
using namespace cv;

pixel::pixel(Vec2i coord)
{
    this->coord = coord;
}

pixel::pixel(cv::Vec2i coord, vectorLine* line, bool isNode)
{
    this->coord = coord;
    this->line = line;
    this->isNode = isNode;
}

pixel::~pixel(){}

bool pixel::operator <(const pixel* other) const
{
    if(this->coord[0] != other->coord[0])
        return this->coord[0] < other->coord[0];

    else
        return this->coord[1] < other->coord[1];
}


// --------------------------------------------------


vectorLine::vectorLine()
{
    this->pixels = vector<pixel*>();
    this->start = NULL;
    this->end = NULL;
}

vectorLine::~vectorLine() {}

pixel* vectorLine::getStart()
{
    return start;
}

pixel* vectorLine::getEnd()
{
    return end;
}

void vectorLine::setStart(pixel* pos)
{
    this->start = pos;
}

void vectorLine::setEnd (pixel* pos)
{
    this->end = pos;
}

vector<pixel*> vectorLine::getPixels ()
{
    return pixels;
}

void vectorLine::addPixels (pixel* px)
{
    this->pixels.push_back(px);
}


bool vectorLine::operator < (const vectorLine other) const
{
    // order by start points
    if(this->start->coord != other.start->coord)
    {
        if(this->start->coord[0] != other.start->coord[0])
            return this->start->coord[0] < other.start->coord[0];

        else
            return this->start->coord[1] < other.start->coord[1];
    }

    // if start points are the same, order by
    // ending point.
    else
    {
        if(this->end->coord[0] != other.end->coord[0])
            return this->end->coord[0] < other.end->coord[0];

        else
            return this->end->coord[1] < other.end->coord[1];
    }
}

