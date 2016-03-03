/**
  * A multi-stage vector line identification algorithm
  * based on Hough-transformation.
  *
  * Author: Arthur Gieberlein
  */

#include "include/vectorization/iterative_linematching.hpp"
#include "include/vectorization/zhangsuen.hpp"

#include <math.h>
#include <stack>
#include <list>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace cv;

#define PI 3.14159265

// Negates a binary image (I think bitwise_not(mat) should be easier)
void negateBlackLayer(int threshold, Mat* input, Mat* output)
{
    if (input->channels() > 1)return;

    for (int i = 0; i < input->rows; i++)
    {
        for (int j = 0; j < input->cols; j++)
        {
            uchar currentPixel = input->at<uchar>(i,j);

            // check thresholds (Vec3b is BGR!)
            if (currentPixel <= threshold)
            {
                // pixel below all thresholds: make it white
                output->at<uchar>(i,j) = 255;
            }

            // else: make pixel black
            else
                output->at<uchar>(i,j) = 0;
        }
    }

    namedWindow("negative black layer", WINDOW_AUTOSIZE);
    imshow("negative black layer", *output);
}

/**
* @brief Extract a white layer from an image. If a pixel
* is above the supplied tresholds for each channel, the
* pixel is assumed to be white.
* @param input The input image in matrix form.
* @param output The black layer in matrix form.
*/
void getWhiteLayer(int threshold, cv::Mat* input, cv::Mat* output){

    if (input->channels() > 1)return;

    for (int i = 0; i < input->rows; i++)
    {
        for (int j = 0; j < input->cols; j++)
        {
            uchar currentPixel = input->at<uchar>(i, j);
            // check thresholds (Vec3b is BGR!)
            if (currentPixel >= threshold)
            {
                // pixel above all thresholds: make it white
                output->at<uchar>(i, j) = 255;
            }
            // else: make pixel black
            else
                output->at<uchar>(i, j) = 0;
        }
    }

    namedWindow("white layer", WINDOW_AUTOSIZE);
    imshow("white layer", *output);

}

void extendLine(cv::Mat* src, cv::Vec4i* srcline, float eps){

    Vec4i l = srcline[0]; //wtf?
    Point one = Point(l[0], l[1]);
    Point two = Point(l[2], l[3]);

    float dir = std::atan2(two.y - one.y, two.x - one.x) * 180 / PI;
    Point betterOne = one;
    Point cfo = one;
    bool dirty = true;
    float diff;
    float betterDiff;
    float cfoDiff;
    float cfoDir;

    for (int i = 0; i <= 1; i++){

        if (i == 1){
            one = Point(l[2], l[3]);
            two = Point(l[0], l[1]);
            betterOne = one;
            cfo = one;
            dirty = true;
            dir = std::atan2(two.y - one.y, two.x - one.x) * 180 / PI;
        }

        while (dirty){
            dirty = false;
            diff = norm(two - one);
            betterDiff = norm(betterOne - two);
            for (int u = -1; u <= 1; u++){
                for (int v = -1; v <= 1; v++){
                    cfo = one + Point(u, v);
                    if (!(cfo.x < 0 || cfo.y < 0) && !(cfo.x >= src->cols || cfo.y >= src->rows)){
                        if (src->at<uchar>(cfo) >= 250){
                            cfoDiff = norm(cfo - two);
                            float cfoDir = std::atan2(two.y - cfo.y, two.x - cfo.x) * 180 / PI;
                            if (cfoDiff > diff && cfoDiff > betterDiff && abs(dir - cfoDir) < eps){
                                betterOne = cfo;
                                betterDiff = cfoDiff;
                                dirty = true;
                            }
                        }
                    }
                }
            }
            if (dirty){
                one = betterOne;
            }
        }
        if (i == 0){
            srcline[0][0] = one.x;
            srcline[0][1] = one.y;
        }
        else if (i == 1){
            srcline[0][2] = one.x;
            srcline[0][3] = one.y;
        }
    }
}

void extendAllLines(cv::Mat* src, vector<cv::Vec4i>* lineList, float eps){

    for (int i = 0; i < lineList->size(); i++){
        extendLine(src, &lineList->at(i) , eps);
    }
}

void drawLineCollection(cv::Mat* target, std::vector<cv::Vec4i> lineList, int thickness, Scalar color){
    for (size_t i = 0; i < lineList.size(); i++)
    {
        Vec4i l = lineList[i];
        line(*target, Point(l[0], l[1]), Point(l[2], l[3]), color, thickness, CV_AA);
    }
}

void appendLineCollection(std::vector<cv::Vec4i>* appendTo, std::vector<cv::Vec4i>* appendFrom, bool shouldClear) {
    for (int i = 0; i < appendFrom->size(); i++){
        appendTo->push_back(appendFrom->at(i));
    }
    if (shouldClear){
        appendFrom->clear();
    }
}

bool edgeAlignment(std::vector<cv::Vec4i>* lineCollection, float gapTH, int rows, int cols){

    bool changes = false;
    int collectionSize = lineCollection->size();
    for (size_t i = 0; i < collectionSize; i++){
        Vec4i n = lineCollection->at(i);
        for (int j = i + 1; j < collectionSize; j++){
            Vec4i m = lineCollection->at(j);
            Point nx = Point(n[0], n[1]);
            Point ny = Point(n[2], n[3]);
            Point mx = Point(m[0], m[1]);
            Point my = Point(m[2], m[3]);
            Point poi;



#if 1
            if ((norm(nx - mx) < gapTH && norm(ny - my) < gapTH) || (norm(nx - my) < gapTH && norm(ny - mx) < gapTH)){
                lineCollection->at(j) = lineCollection->at(i);
                //changes = true;

            }else{

                if (norm(nx - mx) < gapTH && norm(nx - mx)!=0){
                    m[0] = n[0];
                    m[1] = n[1];
                    lineCollection->at(j) = m;
                    changes = true;
                }
                else if (norm(mx - ny) < gapTH && norm(nx - mx) != 0){
                    m[0] = n[2];
                    m[1] = n[3];
                    lineCollection->at(j) = m;
                    changes = true;
                }
                else if (norm(my - nx) < gapTH && norm(nx - mx) != 0){
                    m[2] = n[0];
                    m[3] = n[1];
                    lineCollection->at(j) = m;
                    changes = true;
                }
                else if (norm(my - ny) < gapTH && norm(nx - mx) != 0){
                    m[2] = n[2];
                    m[3] = n[3];
                    lineCollection->at(j) = m;
                    changes = true;
                }


            }


#else

            if (norm(nx - mx) < gapTH){
                if (getPointOfIntersection(n, m, &poi) > 0 && (poi.x < cols && poi.y < rows) &&
                        (norm(poi - mx) < gapTH && norm(nx - poi) < gapTH)){

                    m[1] = poi.x;
                    m[0] = poi.y;
                    n[1] = poi.x;
                    n[0] = poi.y;
                    lineCollection->at(j) = m;
                    lineCollection->at(i) = n;


                }
                else{
                    m[0] = n[0];
                    m[1] = n[1];
                    lineCollection->at(j) = m;
                    lineCollection->at(i) = n;
                }

            }
            else if (norm(mx - ny) < gapTH){

                if (getPointOfIntersection(n, m, &poi) > 0 && (poi.x < cols && poi.y < rows) &&
                        (norm(poi - mx) < gapTH && norm(ny - poi) < gapTH)){
                    m[0] = poi.x;
                    m[1] = poi.y;
                    n[2] = poi.x;
                    n[3] = poi.y;
                    lineCollection->at(j) = m;
                    lineCollection->at(i) = n;
                }
                else{
                    m[0] = n[2];
                    m[1] = n[3];
                    lineCollection->at(j) = m;
                    lineCollection->at(i) = n;
                }

            }
            if (norm(my - nx) < gapTH){

                if (getPointOfIntersection(n, m, &poi) > 0 && (poi.x < cols && poi.y < rows) &&
                        (norm(poi - my) < gapTH && norm(nx - poi) < gapTH)){
                    m[2] = poi.x;
                    m[3] = poi.y;
                    n[0] = poi.x;
                    n[1] = poi.y;
                    lineCollection->at(j) = m;
                    lineCollection->at(i) = n;
                }
                else{
                    m[2] = n[0];
                    m[3] = n[1];
                    lineCollection->at(j) = m;
                    lineCollection->at(i) = n;
                }


            }
            else if (norm(my - ny) < gapTH){

                if (getPointOfIntersection(n, m, &poi) > 0 && (poi.x < cols && poi.y < rows) &&
                        (norm(poi - my) < gapTH && norm(ny - poi) < gapTH)){
                    m[2] = poi.x;
                    m[3] = poi.y;
                    n[2] = poi.x;
                    n[3] = poi.y;
                    lineCollection->at(j) = m;
                    lineCollection->at(i) = n;

                }
                else{
                    m[2] = n[2];
                    m[3] = n[3];
                    lineCollection->at(j) = m;
                    lineCollection->at(i) = n;
                }
            }



#endif
        }

    }
    return changes;
}


int getPointOfIntersection(cv::Vec4i L1, cv::Vec4i L2, cv::Point* output){

    /*
    return:
    0 -> parallel
    -1 -> identisch
    1 -> Schnittpunkt vorhanden
    */


    Point a1 = Point(L1[0], L1[1]);
    Point a2 = Point(L1[2], L1[3]);
    Point b1 = Point(L2[0], L2[1]);
    Point b2 = Point(L2[2], L2[3]);


    //nenner
    float denom = ((b2.y- b1.y)*(a2.x- a1.x)) -	((b2.x - b1.x)*(a2.y-a1.y));
    // zaeher 1
    float numeA = ((b2.x - b1.x)*(a1.y - b1.y)) - ((b2.y - b1.y)*(a1.x - b1.x));
    //zaehler 2
    float numeB = ((a2.x - a1.x)*(a1.y - b1.y)) - ((a2.y - a1.y)*(a1.x - b1.x));

    if (denom == 0){
        if (numeA == 0 && numeB == 0){
            return -1;
        }
        return 0;
    }

    float Ua = numeA / denom;
    float Ub = numeB / denom;

    output->x = a1.x + Ua*(a2.x - a1.x);
    output->y = a1.y + Ua*(a2.y - a1.y);
    return 1;

}

void customThinning(cv::Mat* target, int numIterations, int cannyParam){

    Mat tmp;

    if (target->channels() == 1){
        for (int v = 0; v < numIterations; v++){
            target->copyTo(tmp);
            Canny(tmp, tmp, cannyParam * 0.4, cannyParam, 5);
            for (int n = 0; n < target->rows; n++){
                for (int m = 0; m < target->cols; m++){
                    if (tmp.at<uchar>(n, m) == 255){
                        target->at<uchar>(n, m) = 255;
                    }
                }
            }
        }
    }


}

void removeLineFromBlackLayer(cv::Mat* target, vector<cv::Vec4i> lineCollection){

    for (int j = 0; j < lineCollection.size(); j++){
        Vec4i line = lineCollection[j];

        Point A = Point(line[0], line[1]);
        Point B = Point(line[2], line[3]);
        Point lineRunner;

        float eps = 0.005f;

        for (float i = 0.0f; i <= 1.0f; i =  i + eps){

            lineRunner = A + i*(B - A);
            for (int u = -1; u <= 1; u++){
                for (int v = -1; v <= 1; v++){
                    int x = lineRunner.x + u;
                    int y = lineRunner.y + v;

                    if (x>=0 && x < target->cols && y>=0 && y < target->rows){
                        target->at<uchar>(lineRunner + Point(u, v)) =0;
                    }
                }
            }
        }
    }
}

void pointsToLines(cv::Mat* target, cv::Mat* drawTarget, std::vector<cv::Vec4i>* lineCollection){

    int gap = 3;

    Vec4i tempLine = (0, 0, 0, 0);
    std::vector<cv::Vec4i> tempLineCollection, neighbourLines;
    std::vector<cv::Point> tempPointList;

    if (target->channels() > 1)return;

    for (int u = 0; u < target->rows; u++){
        for (int v = 0; v < target->cols; v++){
            if (target->at<uchar>(u, v) == 255){
                target->at<uchar>(u, v) == 0;
                for (int m = -1; m <= 1; m++){
                    for (int n = -1; n <= 1; n++){

                        if ((m != 0 && n != 0) && (u + m) >= 0 && (v + n) >= 0 && (u + m) < target->rows && (v + n) < target->cols){
                            if (target->at<uchar>(u + m, v + n) == 255){
                                tempLineCollection.push_back(Vec4i(u, v, u + m, v + m));
                                target->at<uchar>(u + m, v + n) == 0;
                            }
                            else{
                                tempPointList.push_back(Point(u, v));
                            }
                        }
                    }
                }

            }
        }
    }

    for (int i = 0; i < tempPointList.size(); i++){
        Point actPoint = tempPointList[i];

        for (int j = 0; j < lineCollection->size(); j++){
            Vec4i actLine = lineCollection->at(j);
            Point APoint = Point(actLine[0], actLine[1]);
            Point BPoint = Point(actLine[2], actLine[3]);

            int normPtoA = norm(actPoint - APoint);
            int normPtoB = norm(actPoint - BPoint);

            if (normPtoA <= gap ){
                neighbourLines.push_back(lineCollection->at(j));
            }
            else if (normPtoB <= gap){
                neighbourLines.push_back(Vec4i(BPoint.x, BPoint.y, APoint.x, APoint.y));
            }
        }

        if (neighbourLines.size() >= 2){
            lineCollection->push_back(Vec4i(neighbourLines[0][0], neighbourLines[0][1], neighbourLines[1][0], neighbourLines[1][0]));
        }
        else if (neighbourLines.size() >= 1){
            lineCollection->push_back(Vec4i(neighbourLines[0][0], neighbourLines[0][1], actPoint.x,actPoint.y));
        }

        neighbourLines.clear();
    }

    for (int i = 0; i < tempLineCollection.size(); i++){
        Vec4i actTempLine = tempLineCollection[i];
        Point actPointA = Point(actTempLine[0], actTempLine[1]);
        Point actPointB = Point(actTempLine[2], actTempLine[3]);

        for (int j = 0; j < lineCollection->size(); j++){
            Vec4i actMainLine = lineCollection->at(j);
            Point APoint = Point(actMainLine[0], actMainLine[1]);
            Point BPoint = Point(actMainLine[2], actMainLine[3]);

            int normTAtoMA = norm(actPointA - APoint);
            int normTAtoMB = norm(actPointA - BPoint);
            int normTBtoMA = norm(actPointB - APoint);
            int normTBtoMB = norm(actPointB - BPoint);

            if (normTAtoMA <= gap){
                neighbourLines.push_back(Vec4i(actPointA.x, actPointA.y, APoint.x, APoint.y));
            }
            else if (normTAtoMB <= gap){
                neighbourLines.push_back(Vec4i(actPointA.x, actPointA.y, BPoint.x, BPoint.y));
            }
            else if (normTBtoMA <= gap){
                neighbourLines.push_back(Vec4i(actPointB.x, actPointB.y, APoint.x, APoint.y));
            }
            else if (normTAtoMB <= gap){
                neighbourLines.push_back(Vec4i(actPointB.x, actPointB.y, BPoint.x, BPoint.y));
            }
        }


    }

    appendLineCollection(lineCollection, &neighbourLines);
}

void getFinalLines(cv::Mat* blacklayer, std::vector<cv::Vec4i>* dirLineCollection, char* source_window, bool debug){


    Mat cMat;// 3 channel mat from blacklayer to draw red lines in
    Mat gMat = Mat(blacklayer->rows, blacklayer->cols, CV_8U);
    Mat nBlacklayer;
    vector<Vec4i> lines;
    //vector<Vec4i> finalLineCollection;

    for (int i = 1; i < 5; i = i + 2)
    {
        //medianBlur(*blacklayer, *blacklayer, i);
    }

    nBlacklayer = blacklayer->clone();
    negateBlackLayer(100, &nBlacklayer, &nBlacklayer);


    negateBlackLayer(100, blacklayer, blacklayer);
    thinning(*blacklayer);
    cv::cvtColor(*blacklayer, cMat, CV_GRAY2BGR);
    cv::cvtColor(cMat, gMat, CV_BGR2GRAY);



    //1. ITERATION =====================================================
    HoughLinesP(*blacklayer, lines, 2, CV_PI / 720, 50, 20, 5);
    extendAllLines(blacklayer, &lines, 0.1);
    //edgeAlignment(&lines, 5, blacklayer->cols, blacklayer->rows);
    drawLineCollection(&cMat, lines, 1, Scalar(0, 0, 255));
    if (debug){ cv::imshow(source_window, cMat); cv::waitKey(0); }
    cv::cvtColor(cMat, gMat, CV_BGR2GRAY);
    getWhiteLayer(250, &gMat, &gMat);
    cv::cvtColor(gMat, cMat, CV_GRAY2BGR);
    if (debug){ cv::imshow(source_window, cMat); cv::waitKey(0); }
    appendLineCollection(dirLineCollection, &lines, true);

    //2.ITERATION=======================================================
    HoughLinesP(gMat, lines, 1, CV_PI / 180, 20, 10, 3);
    extendAllLines(&gMat, &lines, 0.1);
    //edgeAlignment(&lines, 2, blacklayer->cols, blacklayer->rows);
    drawLineCollection(&cMat, lines, 1, Scalar(0, 255, 255));
    if (debug){ cv::imshow(source_window, cMat); cv::waitKey(0); }
    cv::cvtColor(cMat, gMat, CV_BGR2GRAY);
    getWhiteLayer(250, &gMat, &gMat);
    cv::cvtColor(gMat, cMat, CV_GRAY2BGR);
    if (debug){ cv::imshow(source_window, cMat); cv::waitKey(0); }
    appendLineCollection(dirLineCollection, &lines, true);

    //3. ITERATION========================================================
    HoughLinesP(gMat, lines, 1, CV_PI / 180, 4, 3, 2);
    extendAllLines(&gMat, &lines, 0.1);
    //edgeAlignment(&lines, 5, blacklayer->cols, blacklayer->rows);
    drawLineCollection(&cMat, lines, 1, Scalar(0, 255, 255));
    if (debug){ cv::imshow(source_window, cMat); cv::waitKey(0); }
    cv::cvtColor(cMat, gMat, CV_BGR2GRAY);
    getWhiteLayer(250, &gMat, &gMat);
    cv::cvtColor(gMat, cMat, CV_GRAY2BGR);
    if (debug){ cv::imshow(source_window, cMat); cv::waitKey(0); }
    appendLineCollection(dirLineCollection, &lines, true);

    Mat old, diff;
    bool equal = false;

    //4. ITERATION========================================================
    while (!equal){
        old = gMat.clone();
        HoughLinesP(gMat, lines, 1, CV_PI / 180, 2, 2, 2);
        extendAllLines(&gMat, &lines, 20);
        drawLineCollection(&cMat, lines, 1, Scalar(0, 255, 255));
        //edgeAlignment(&lines, 5, blacklayer->cols, blacklayer->rows);
        appendLineCollection(dirLineCollection, &lines, true);
        cv::cvtColor(cMat, gMat, CV_BGR2GRAY);
        getWhiteLayer(250, &gMat, &gMat);
        cv::cvtColor(gMat, cMat, CV_GRAY2BGR);

        diff = old != gMat;
        equal = countNonZero(diff) == 0;
    }

    //FINAL ITERATION========================================================

    cv::cvtColor(*blacklayer, cMat, CV_GRAY2BGR);
    extendAllLines(blacklayer, dirLineCollection, 0.1);
    drawLineCollection(&cMat, *dirLineCollection, 1, Scalar(0, 0, 255));
    edgeAlignment(dirLineCollection, 2, blacklayer->cols, blacklayer->rows);
    if (debug){ cv::imshow(source_window, cMat); cv::waitKey(0); }
    cv::cvtColor(cMat, gMat, CV_BGR2GRAY);
    getWhiteLayer(250, &gMat, &gMat);
    cv::cvtColor(gMat, cMat, CV_GRAY2BGR);

    equal = false;
    while (!equal){
        old = gMat.clone();
        HoughLinesP(gMat, lines, 1, CV_PI / 720, 1, 1, 3);
        extendAllLines(&gMat, &lines, 20);
        drawLineCollection(&cMat, lines, 1, Scalar(0, 255, 255));
        //edgeAlignment(&lines, 5, blacklayer->cols, blacklayer->rows);
        appendLineCollection(dirLineCollection, &lines, true);
        cv::cvtColor(cMat, gMat, CV_BGR2GRAY);
        getWhiteLayer(250, &gMat, &gMat);
        cv::cvtColor(gMat, cMat, CV_GRAY2BGR);

        diff = old != gMat;
        equal = countNonZero(diff) == 0;
    }

    edgeAlignment(dirLineCollection, 3, blacklayer->cols, blacklayer->rows);


    //DRAW LINE COLLECTION====================================================

    drawLineCollection(&cMat, *dirLineCollection, 1, Scalar(0, 255, 255));
    cv::imshow(source_window, cMat);
    //cv::waitKey(0);

    if (debug){
        cv::cvtColor(cMat, gMat, CV_BGR2GRAY);
        getWhiteLayer(250, &gMat, &gMat);
        cv::cvtColor(gMat, cMat, CV_GRAY2BGR);

        cv::imshow(source_window, cMat);
        cv::waitKey(0);
    }

}
