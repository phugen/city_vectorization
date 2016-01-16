#ifndef VEC2ICOMPARE_HPP
#define VEC2ICOMPARE_HPP

#include "include/opencvincludes.hpp"


// Functor for std::map and Vec2i keys.
// Defines the order of vectors by using their row element.
struct Vec2iCompare
{
    bool operator () (const cv::Vec2i a, const cv::Vec2i b) const
    {
        return a[0] < b[0];
    }
};

#endif // VEC2ICOMPARE_HPP

