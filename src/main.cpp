/**
  * A program that takes an image, deletes text from it
  * and then tries to convert it into a vector representation.
  *
  * Author: phugen
  */

#include "include/opencvincludes.hpp"
#include "include/auxiliary.hpp"
#include "include/unionfindcomponents.hpp"
#include "include/areafilter.hpp"
#include "include/collineargrouping.hpp"
#include "include/zhangsuen.hpp"
#include "include/vectorize.hpp"

#include <iostream>
#include <algorithm>


using namespace std;
using namespace cv;


int main (int argc, char** argv)
{
    Mat original, *output;
    vector<ConnectedComponent> components;

    //original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/CV_sample_schwer_2.png", CV_LOAD_IMAGE_COLOR);
    original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/vectortest.png", CV_LOAD_IMAGE_COLOR);
    //original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/CV_sample_mittel.png", CV_LOAD_IMAGE_COLOR);


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
    collinearGrouping(*output, output, &components);
    vectorizeImage(output, "vectorized");

    waitKey(0);
}
