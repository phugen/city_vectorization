/**
  * A program that takes an image, deletes text from it
  * and then tries to convert it into a vector representation.
  *
  * Author: phugen
  */

#include "include/opencvincludes.hpp"
#include "include/text_segmentation/auxiliary.hpp"
#include "include/text_segmentation/unionfindcomponents.hpp"
#include "include/text_segmentation/areafilter.hpp"
#include "include/text_segmentation/collineargrouping.hpp"
#include "include/vectorization/zhangsuen.hpp"
#include "include/vectorization/vectorize.hpp"
#include "include/vectorization/iterative_linematching.hpp"

#include <iostream>
#include <algorithm>
#include <time.h>


using namespace std;
using namespace cv;


int main (int argc, char** argv)
{
    int start_time = time(NULL);


    Mat original, process, output;
    vector<ConnectedComponent> components;

    // load image here
    original = imread("../../cityplan_vectorization/presentation_IMAGE.png");

    if(!original.data)
    {
        cout << "The image couldn't be loaded. Maybe the file name was wrong?\n";
        return -1;
    }

    process = original.clone();
    output = Mat(original.rows, original.cols, CV_8U); // output matrix

    cout << original.depth() << " " << original.channels() << "\n";


    // TODO: Automatically choosing reasonable threshold?
    Vec3b thresholds = Vec3b(180, 180, 180);

    getBlackLayer(thresholds, process, &output); // black layer creation
    unionFindComponents(&output, &components, 10); // MBR detection
    areaFilter(&components, 10); // ratio component filtering
    collinearGrouping(output, &output, &components); // text removal
    vectorizeImage(&output, &original, "vectorized", 2); // vectorization of image

    int end_time = time(NULL);
    printf ("\n\nVectorizing this file took %d second(s)!\n", (end_time - start_time));

    waitKey(0);
}
