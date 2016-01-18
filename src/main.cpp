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

    original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/CV_sample_schwer_2.png", CV_LOAD_IMAGE_COLOR);
    //original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/peter.png", CV_LOAD_IMAGE_COLOR);
    //original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/CV_sample_schwer.png", CV_LOAD_IMAGE_COLOR);


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
    unionFindComponents(*output, &components);
    areaFilter(&components, 10);
    collinearGrouping(*output, &components);

    /*bitwise_not(*output, *output); // algorithm expects binary picture with black background
    // find corners; find a way to describe min distance parameter dynamically if possible!
    //thinning(*output); // skeletonize image (1px lines only)
    goodFeaturesToTrack(*output, corners, output->cols*output->rows, 0.1, 0 );


    Mat reconstructed = Mat::zeros(original.rows, original.cols, original.type());
    reconstructed = Scalar(255, 255, 255); // for showing reconstructed polygons

    Mat cornerMat = *output; // for showing circles around detected corners
    cvtColor(cornerMat, cornerMat,CV_GRAY2RGB);
    bitwise_not(cornerMat, cornerMat);

    // show corners
    for(size_t i = 0; i < corners.size(); i++)
    {
        cout << "CORNER: " << corners[i] << "\n";
        //circle(cornerMat, corners[i], 10, Scalar(0, 255, 0), 1);
        rectangle(cornerMat, corners[i], corners[i], Scalar(0, 0, 255), 3, 8, 0);
    }

    namedWindow("CORNERS", WINDOW_AUTOSIZE);
    imshow("CORNERS", cornerMat);

    // find reachable corners for every point
    bitwise_not(*output, *output);
    vcorners = pointToVec(corners);

    for(auto iter = vcorners.begin(); iter != vcorners.end(); iter++)
    {
        Vec2i current = *iter;
        vector<Vec2i> currNeigh = getNearestCorners(vcorners, current, output, &reconstructed);
    }

    namedWindow("RECONSTRUCTED", WINDOW_AUTOSIZE);
    imshow("RECONSTRUCTED", reconstructed);*/

    //vector<Vec2f> line (0,0);
    //line.insert(line.begin(), Vec2f(50, 0.785398));
    //drawLines(line, output, Scalar(0, 255, 0));

    //namedWindow("linetest", WINDOW_AUTOSIZE);
    //imshow("linetest", *output);


    waitKey(0);
}
