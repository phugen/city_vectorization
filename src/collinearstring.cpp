/**
  * A class containing information about a collinear string,
  * as detected by the algrorithm described in "collinearGrouping.cpp".
  *
  * Author: phugen
  */

#include "include/collinearstring.hpp"
#include "include/auxiliary.hpp"

using namespace std;
using namespace cv;

#include <iostream>

CollinearString::CollinearString(vector<ConnectedComponent> cluster)
{
    comps = cluster;

    refine();
}

// Calculates the local average height from the four neighbors
// of the given component (if possible).
// Depending on the orientation of the cluster line, either
// the component's horizontal or vertical MBR lines are used
// for the calculation.
double CollinearString::localAvgHeight (vector<ConnectedComponent> cluster, int listPos)
{
    int startPos, endPos;
    double localAvg = 0;
    int dim;


    // account for comps near the start or end of the cluster list
    listPos - 2 < 0 ? startPos = 0 : startPos = listPos - 2;
    listPos + 2 >= (int) cluster.size() ? endPos = (int) cluster.size() - 1 : endPos = listPos + 2;

    // determine which MBR dimension to use by inspecting line angle
    // [0°, 45°] = horizontal, vertical otherwise
    if(cluster.at(listPos).houghLine[1] <= 0.785398)
        dim = 0;
    else
        dim = 1;

    // calculate average height
    for(int i = startPos; i <= endPos; i++)
        localAvg += cluster.at(i).mbr_max[dim] - cluster.at(i).mbr_min[dim];


    return (localAvg /= endPos - startPos);
}


// Calculates the distance from the component at listPos
// to its successor in the list, if possible.
double CollinearString::edgeToEdgeDistance (vector<ConnectedComponent> cluster, int listPos)
{
    // This is the last component in the list.
    // Distance should already have been calculated
    // by edgeToEdgeDistance(cluster, listPos - 1).
    if(listPos == (int) cluster.size() - 1)
        return INT_MAX; // dummy value

    /**
     * TODO: CHECK IF POINT COORDINATES ARE RIGHT - "Matrix style" coordinates?*/

    // First MBR
    Vec2i mbr_min = cluster.at(listPos).mbr_min;
    Vec2i mbr_max = cluster.at(listPos).mbr_max;

    Vec2i bot_left_1 = mbr_min;
    Vec2i bot_right_1 = Vec2i(mbr_min[0], mbr_max[1]);
    Vec2i top_right_1 = mbr_max;
    Vec2i top_left_1 = Vec2i(mbr_max[0], mbr_min[1]);

    // Second MBR
    Vec2i mbr_min2 = cluster.at(listPos + 1).mbr_min;
    Vec2i mbr_max2 = cluster.at(listPos + 1).mbr_max;

    Vec2i bot_left_2 = mbr_min2;
    Vec2i bot_right_2 = Vec2i(mbr_min2[0], mbr_max2[1]);
    Vec2i top_right_2 = mbr_max2;
    Vec2i top_left_2 = Vec2i(mbr_max2[0], mbr_min2[1]);

    // define line endpoints for both MBRs
    pair<Vec2i, Vec2i> left_1 = make_pair(bot_left_1, top_left_1);
    pair<Vec2i, Vec2i> bottom_1 =  make_pair(bot_left_1, bot_right_1);
    pair<Vec2i, Vec2i> right_1 =  make_pair(bot_right_1, top_right_1);
    pair<Vec2i, Vec2i> top_1 =  make_pair(top_right_1, top_left_1);

    pair<Vec2i, Vec2i> left_2 =  make_pair(bot_left_2, top_left_2);
    pair<Vec2i, Vec2i> bottom_2 =  make_pair(bot_left_2, bot_right_2);
    pair<Vec2i, Vec2i> right_2 =  make_pair(bot_right_2, top_right_2);
    pair<Vec2i, Vec2i> top_2 =  make_pair(top_right_2, top_left_2);

    // First test if MBRs overlap - if they do, we'll consider the distance zero.

    //if (RectA.Left < RectB.Right && RectA.Right > RectB.Left &&
    //     RectA.Top > RectB.Bottom && RectA.Bottom < RectB.Top )

    //if (RectA.X1 < RectB.X2 && RectA.X2 > RectB.X1 &&
    //    RectA.Y1 < RectB.Y2 && RectA.Y2 > RectB.Y1)

    // Check Y-aligned lines: m = +infinity => x = a
    if(left_1.first[1] < right_2.first[1] && right_1.first[1] > left_2.first[1])
        return 0.;

    // Check X-aligned lines: m = 0 => y = b
    if(top_1.first[0] > bottom_2.first[0] && bottom_1.first[0] < top_2.first[0])
        return 0.;

    // The rectangles do not intersect, so find the minimum distance between the two
    // rectangles. The shortest line always has its origin on a vertex of one of
    // the two rectangles, so calculate the distances of the perpendicular line
    // segments that start at all vertices and end at each of the four sides
    // of the rectangle, then choose the minimum one out of those.
    //
    // (Could be improved by figuring out which line divides the two rectangles, so
    // the two/three "back side" lines could be culled from the calculation)
    vector<Vec2i> mbr1_points { bot_left_1, bot_right_1, top_right_1, top_left_1 };
    vector<Vec2i> mbr2_points { bot_left_2, bot_right_2, top_right_2, top_left_2 };

    vector<pair<Vec2i, Vec2i>> mbr1_sides { left_1, bottom_1, right_1, top_1 };
    vector<pair<Vec2i, Vec2i>> mbr2_sides { left_2, bottom_2, right_2, top_2 };

    double min_dist = DBL_MAX;

    // Distances from points of MBR1 to sides of MBR2
    for(auto p1 = mbr1_points.begin(); p1 != mbr1_points.end(); p1++)
            for(auto side2 = mbr2_sides.begin(); side2 != mbr2_sides.end(); side2++)
                min_dist = min(min_dist, distanceFromCartesianLine(*p1, *side2));

    // Distances from points of MBR2 to sides of MBR1
    for(auto p2 = mbr2_points.begin(); p2 != mbr2_points.end(); p2++)
        for(auto side1 = mbr1_sides.begin(); side1 != mbr1_sides.end(); side1++)
            min_dist = min(min_dist, distanceFromCartesianLine(*p2, *side1));

    return min_dist;
}

void CollinearString::refine ()
{
    if(comps.size() == 0)
        return;

    double avgHeight, distToNext;

    int oldsize = comps.size();
    int newsize = -1;

    // refine until stable - probably not needed
    while( oldsize != newsize)
    {
        oldsize = comps.size();

        for(auto comp = comps.begin(); comp != comps.end() - 1; comp++)
        {
            int listpos = comp - comps.begin();

            // deletion invalidated position
            if(listpos >= (int) comps.size())
                break;

            // Calculate average local height and distance to the neighboring component
            avgHeight = localAvgHeight(comps, listpos);
            distToNext = edgeToEdgeDistance(comps, listpos);
            //distToNext = distanceBetweenPoints (comps.at(listpos).centroid, comps.at(listpos+1).centroid);

            // if the inter-character distance is higher
            // than the threshold, don't delete this component
            if(distToNext > avgHeight)
                comps.erase(comp);
        }

        newsize = comps.size();
    }
}
