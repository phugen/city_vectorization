#include "include/opencvincludes.hpp"
#include "include/auxiliary.hpp"
#include "include/unionfindcomponents.hpp"
#include "include/areafilter.hpp"
#include "include/collineargrouping.hpp"
#include "include/zhangsuen.hpp"

#include <iostream>
#include <algorithm>


using namespace std;
using namespace cv;


int main (int argc, char** argv)
{
    Mat original, *output;
    vector<ConnectedComponent> components;

    //original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/CV_sample_schwer_2.png", CV_LOAD_IMAGE_COLOR);
    //original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/peter.png", CV_LOAD_IMAGE_COLOR);
    original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/houghtest.png", CV_LOAD_IMAGE_COLOR);


    if(!original.data)
    {
        cout << "The image couldn't be loaded. Maybe the file name was wrong?\n";
        return -1;
    }

    output = new Mat(original.rows, original.cols, CV_8U); // output matrix
    vector<Point2i> corners;
    vector<Vec2i> vcorners;


    // BGR format
    // Thresholds need testing with unscaled images to avoid
    // artifacts artificially increasing the needed threshold
    //
    // TODO: Slider adjustment for black layer
    Vec3b thresholds = Vec3b(180, 180, 180);

    getBlackLayer(thresholds, original, output);
    unionFindComponents(output, &components, 0); // size: 10
    areaFilter(&components, 10);

    namedWindow("test");
    imshow("test", *output);
    //waitKey(0);

    collinearGrouping(*output, &components);

    //cout << "DIST: " << distanceFromPolarLine(Vec2f(0, 0), Vec2f(100, 1.57)) << "\n";


   /* vector<Vec2f> lines; lines.push_back(Vec2f(100, 1.53589));
    Mat test = Mat::zeros(400, 600, CV_8UC3);
    drawLines(lines, &test, Scalar(0, 255, 0));
    namedWindow("LINETEST");
    imshow("LINETEST", test); */

    waitKey(0);
}
