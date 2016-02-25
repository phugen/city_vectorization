#include "include/vectorline.hpp"

using namespace std;
using namespace cv;

vectorLine::vectorLine() {}
vectorLine::~vectorLine() {}

Vec2i vectorLine::getStart()
{
    return start;
}

Vec2i vectorLine::getEnd()
{
    return end;
}

void vectorLine::setStart(Vec2i pos)
{
    this->start = pos;
}

void vectorLine::setEnd (Vec2i pos)
{
    this->end = pos;
}

vector<Vec2i> vectorLine::getPixels ()
{
    return pixels;
}

void vectorLine::addPixels (Vec2i px)
{
    this->pixels.push_back(px);
}

void vectorLine::addPixels (vector<Vec2i>::iterator start, vector<Vec2i>::iterator end)
{
    for(auto iter = start; iter != end; iter++)
    {
        this->pixels.push_back(*iter);
    }
}

bool vectorLine::operator < (const vectorLine other) const
{
    // order by start points
    if(this->start != other.start)
    {
        if(this->start[0] != other.start[0])
            return this->start[0] < other.start[0];

        else
            return this->start[1] < other.start[1];
    }

    // if start points are the same, order by
    // ending point.
    else
    {
        if(this->end[0] != other.end[0])
            return this->end[0] < other.end[0];

        else
            return this->end[1] < other.end[1];
    }
}

