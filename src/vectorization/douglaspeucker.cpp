#include "include/vectorization/douglaspeucker.h"
#include "include/text_segmentation/auxiliary.hpp"

using namespace std;

// Ramer-Douglas-Peucker algorithm for removing unnecessary nodes
// from a path.
// Epsilon is the maximum error parameter (the smaller it is,
// the less leeway there is for simplifying the line).
//
// To recreate the path, connect the pixels in the order they are
// placed in the return vector.
vector<pixel*> douglasPeucker (vector<pixel*> path, double epsilon)
{
    double dmax = 0.;
    int index = 0;
    int end = path.size() - 1;

    // find index of the point with largest distance from line through first and last in list
    for(int i = 1; i < end; i++)
    {
        double d = distanceFromCartesianSegment(path.at(i)->coord, make_pair(path.at(0)->coord, path.at(end)->coord));

        if(d > dmax)
        {
            index = i;
            dmax = d;
        }
    }

    // keep this worst point

    vector<pixel*> simplifiedPath;

    // If max distance is greater than epsilon, recursively simplify
    if (dmax > epsilon)
    {
        // divide: recursively simplify both subparts of the path
        vector<pixel*> left = douglasPeucker(vector<pixel*>(path.begin(), path.begin() + (index+1)), epsilon);
        vector<pixel*> right = douglasPeucker(vector<pixel*>(path.begin() + index, path.end()), epsilon);

        // conquer: merge sublists        
        left.insert(left.end(), right.begin(), right.end());
        simplifiedPath = left;
    }

    // remove all inner nodes since they are not
    // necessary to characterize the line
    else
    {
        // no inner nodes
        if(path.size() == 2)
            simplifiedPath = path;

        else
        {
            simplifiedPath.push_back(path.at(0));
            simplifiedPath.push_back(path.at(end));
        }
    }

    return simplifiedPath;
}
