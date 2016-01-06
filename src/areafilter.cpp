#include "include/areafilter.hpp"
#include <iostream>

using namespace std;
using namespace cv;

// Dismiss any components that have an area ratio
// less than 1:ratio or larger than ratio:1 because they
// are likely to not be characters.
void areaFilter(Mat input, vector<ConnectedComponent>* components, int ratio)
{
    vector<ConnectedComponent>::iterator iter = components->begin();
    while (iter != components->end())
    {
        ConnectedComponent curr = *iter;

        float x = (curr.mbr_max[0] + 1) - curr.mbr_min[0];
        float y = (curr.mbr_max[1] + 1) - curr.mbr_min[1];

        // if the ratio is less than 1:20 or larger than 20:1
        // consider this component a non-character component
        if((ratio * x) < y || x > (ratio * y))
            iter = components->erase(iter);

        else
            iter++;
    }

    cout << "#Components after area filter: " << components->size() << "\n";

    for(vector<ConnectedComponent>::iterator iter = components->begin(); iter != components->end(); iter++)
    {
        Point min = Vec2i((*iter).mbr_min[1], (*iter).mbr_min[0]);
        Point max = Vec2i((*iter).mbr_max[1], (*iter).mbr_max[0]);

        rectangle(input, min, max, Scalar(0, 0, 255), 1, 8, 0);
    }

    // show result
    //namedWindow("AREA FILTER", WINDOW_AUTOSIZE);
    //imshow("AREA FILTER", input);
}
