#pragma once

#include <include/text_segmentation/connectedcomponent.hpp>

typedef struct parametricLine parametricLine;

struct parametricLine
{
    double m;
    double b;
};

template<typename T> double median (std::vector<T> elements);
parametricLine leastSquaresLine(std::vector<ConnectedComponent>);
parametricLine leastMedianSquaresLine(std::vector<ConnectedComponent>, float p, float t);

