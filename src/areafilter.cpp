/**
  * Implements a filter for connected component MBRs,
  * as needed by the text extraction algorithm in "collineargrouping.cpp".
  *
  * Author: Phugen
  */

#include "include/areafilter.hpp"
#include "include/auxiliary.hpp"
#include <iostream>

using namespace std;
using namespace cv;



// Dismiss any components that have an area ratio
// less than 1:ratio or larger than ratio:1 because they
// are likely to not be characters.
void areaFilter(vector<ConnectedComponent>* components, int ratio)
{
    vector<ConnectedComponent>::iterator iter = components->begin();
    while (iter != components->end())
    {
        ConnectedComponent curr = *iter;

        float x = (curr.mbr_max[0] + 1) - curr.mbr_min[0];
        float y = (curr.mbr_max[1] + 1) - curr.mbr_min[1];

        // if the ratio is less than 1:ratio or larger than ratio:1
        // consider this component a non-character component
        if((ratio * x) < y || x > (ratio * y))
            iter = components->erase(iter);

        else
            iter++;
    }

    //cout << "#Components after area filter: " << components->size() << "\n";

    // show result
    //namedWindow("AREA FILTER", WINDOW_AUTOSIZE);
    //imshow("AREA FILTER", input);
}

// Auxiliary function for sorting a container of ConnectedComponents
// by the area sizes of the component MBRs.
bool sortByMBRArea (ConnectedComponent a, ConnectedComponent b)
{
    return a.area < b.area;
}

// Remove components from this cluster until the ratio from the largest
// to the smallest is equal to or smaller than the input ratio (or the
// cluster consists of only one element).
void clusterCompAreaFilter(vector<ConnectedComponent>* cluster, int ratio)
{
    // Nonsensical ratio
    if(ratio < 1)
        { cout << "clusterCompAreaFilter: Ratio " << ratio << " is < 1!\n"; return; }

    // no ratio filtering needed
    if(cluster->size() == 0 || cluster->size() == 1)
        return;

    // sort list by MBR area sizes
    sort(cluster->begin(), cluster->end(), sortByMBRArea);

    // while the max_area / min_area is > ratio,
    // mark elements for deletion
    auto to = cluster->begin();
    auto from = cluster->end() - 1;

    // Drop components from the cluster until the ratio is obeyed.
    // Always drop either the minimum or maximum element, depending
    // on which has the larger difference to its neighbor.
    while((cluster->at(from - cluster->begin()).area) / (cluster->at(to - cluster->begin()).area) > ratio &&
           to != from)
    {
        // find out which outlier has a greater local difference
        // and mark it for deletion
        int lowDiff = localAreaDiff(*cluster, (to - cluster->begin()), false); //(*(to + 1)).area - (*to).area;
        int highDiff = localAreaDiff(*cluster, (from - cluster->begin()), true); //(*from).area - (*(from - 1)).area;

        if(lowDiff > highDiff)
            to++;

        else
            from--;
    }

    // get subvector excluding those components
    // that were marked for deletion
    *cluster = vector<ConnectedComponent>(to, from + 1);
}
