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

// Remove components from this string until the ratio from the largest
// to the smallest is equal to or smaller than the input ratio.
void clusterCompAreaFilter(vector<ConnectedComponent>* cluster, int ratio)
{
    // TODO: Change the algorithm so that the list just sorted by area
    // size and truncate the last element until the ratio is below the
    // threshold, because that way recalculating the area for every re-
    // maining component becomes unnecessary.

    while(cluster->size() > 1)
    {
        double min_area = DBL_MAX;
        double max_area = -1;

        // position of maximum area component in the cluster list
        vector<ConnectedComponent>::iterator maxPos;

        // find minimum and maximum area in this component
        for(auto comp = cluster->begin(); comp != cluster->end(); comp++)
        {
            // calculate component MBR area size
            Vec2i pmin = (*comp).mbr_min;
            Vec2i pmax = (*comp).mbr_max;

            double area;

            // component consists of only one pixel
            if(pmin == pmax)
                area = 1;

            else
            {
                double len_A = sqrt(pow(pmin[1] - pmax[1], 2) + pow(pmin[0] - pmin[0], 2));
                double len_B = sqrt(pow(pmin[1] - pmin[1], 2) + pow(pmin[0] - pmax[0], 2));

                // Account for cases in which the MBR is a 1 px line
                if(len_A == 0)
                    area = len_B;

                else if (len_B == 0)
                    area = len_A;

                // otherwise: calculate MBR area normally
                else
                    area = len_A * len_B;
            }


            // update min/max area if needed
            // if max area is updated, save position of element
            min_area = min(min_area, area);

            if(area > max_area)
            {
                max_area = area;
                maxPos = comp;
            }
        }

        // erase maximum area element if ratio is wrong
        if(max_area / min_area > ratio)
            cluster->erase(maxPos);

        else
            break;
    }

}
