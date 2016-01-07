#include "include/opencvincludes.hpp"
#include "include/auxiliary.hpp"
#include "include/unionfindcomponents.hpp"
#include "include/areafilter.hpp"
#include "include/collineargrouping.hpp"
#include "include/zhangsuen.hpp"

#include <iostream>


using namespace std;
using namespace cv;


int main (int argc, char** argv)
{
    Mat original, *output;
    vector<ConnectedComponent> components;

    //original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/CV_sample_schwer_2.png", CV_LOAD_IMAGE_COLOR);
    //original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/peter.png", CV_LOAD_IMAGE_COLOR);
    original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/thintest.png", CV_LOAD_IMAGE_COLOR);

    output = new Mat(original.rows, original.cols, CV_8U); // output matrix
    vector<Point2i> corners;
    vector<Vec2i> vcorners;


    // BGR format
    // Thresholds need testing with unscaled images to avoid
    // artifacts artificially increasing the needed threshold
    //Vec3b thresholds = Vec3b(160, 160, 160);
    Vec3b thresholds = Vec3b(180, 180, 180);

    getBlackLayer(thresholds, original, output);
    //unionFindComponents(*output, &components);
    //areaFilter(*output, &components, 10);
    //collinearGrouping(*output, &components);

    bitwise_not(*output, *output); // algorithm expects binary picture with black background
    thinning(*output);

    // find corners; find a way to describe min distance parameter dynamically if possible!
    goodFeaturesToTrack(*output, corners, output->cols*output->rows, 0.1, 30. );

    // show corners
    for(size_t i = 0; i < corners.size(); i++)
        circle(*output, corners[i], 10, Scalar(255, 255, 255), 2);

    // find reachable corners for every point
    vcorners = pointToVec(corners);
    bitwise_not(*output, *output);

    Mat testimage = Mat::zeros(original.rows, original.cols, original.type());
    testimage = Scalar(255, 255, 255);
    for(auto iter = vcorners.begin(); iter != vcorners.end(); iter++)
    {
        Vec2i current = *iter;

        vcorners.erase(iter);
        vector<Vec2i> currNeigh = getNearestCorners(vcorners, current, output);

        // debug lines
        for(auto iter = currNeigh.begin(); iter != currNeigh.end(); iter++)
        {
            int iterno = iter - currNeigh.begin();

            line(testimage, Point(current), Point(*iter), Scalar(0,0,0), 1);
            //putText(testimage, to_string(iterno), current, FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0,255,0), 1, 8, false);
        }
    }


    namedWindow("CORNERS", WINDOW_AUTOSIZE);
    imshow("CORNERS", *output);

    namedWindow("RECONSTRUCTED", WINDOW_AUTOSIZE);
    imshow("RECONSTRUCTED", testimage);


    waitKey(0);
}
