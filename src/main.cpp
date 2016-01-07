#include "include/opencvincludes.hpp"
#include "include/auxiliary.hpp"
#include "include/unionfindcomponents.hpp"
#include "include/areafilter.hpp"
#include "include/collineargrouping.hpp"

#include <iostream>


using namespace std;
using namespace cv;


int main (int argc, char** argv)
{
    Mat original, *output;
    vector<ConnectedComponent> components;

    original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/CV_sample_schwer_2.png", CV_LOAD_IMAGE_COLOR);
    //original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/peter.png", CV_LOAD_IMAGE_COLOR);
    //original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/jointest.png", CV_LOAD_IMAGE_COLOR);
    //original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/erasetest.png", CV_LOAD_IMAGE_COLOR);

    // output matrix will be unsigned 8-bit (use uchar instead of Vec3b)
    output = new Mat(original.rows, original.cols, CV_8U);

    // BGR format
    // Thresholds need testing with unscaled images to avoid
    // artifacts artificially increasing the needed threshold
    //Vec3b thresholds = Vec3b(160, 160, 160);
    Vec3b thresholds = Vec3b(180, 180, 180);

    getBlackLayer(thresholds, original, output); // get binary picture via thresholding
    unionFindComponents(*output, &components); // get connected components (graphics, letters, combination of both)
    areaFilter(*output, &components, 10); // filter out components with extreme dimensions (REVIEW - combination?)
    //collinearGrouping(*output, &components); // cluster strings and remove them from the image

    for(vector<ConnectedComponent>::iterator c = components.begin(); c != components.end(); c++)
        eraseComponentPixels(*c, output);

    namedWindow("ERASED", WINDOW_AUTOSIZE);
    imshow("ERASED", *output);

    waitKey(0);
}
