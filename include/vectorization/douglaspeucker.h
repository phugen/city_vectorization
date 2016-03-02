#pragma once

#include "include/vectorization/vectorline.hpp"
#include <vector>

std::vector<pixel*> douglasPeucker (std::vector<pixel*> path, double epsilon);
