#define _USE_MATH_DEFINES

#include "opencvincludes.hpp"
#include "connectedcomponent.hpp"
#include "colorconversions.hpp"
#include "unionfind.hpp"
#include "customhoughtransform.hpp"

#include <iostream>
#include <list>
#include <set>
#include <array>
#include <vector>
#include <cmath>

#include <random>
#include <ctime>

using namespace std;
using namespace cv;


// maps an integer range to RGB color vectors.
Vec3i intToRGB (Vec2i range, int value)
{
    //float min = (float) range[0];
    float max = (float) range[1];

    int h = (360 / max) * value;

    Vec3i hsv = Vec3i(h, 1, 1);

    return HSVtoRGB(hsv);
}

// draw a line given in polar coordinates on the input image using the given color.
void drawLines (std::vector<Vec2f> lines, cv::Mat* image, Scalar color)
{
    for(size_t i = 0; i < lines.size(); i++)
    {
        float rho = lines[i][0];
        float theta = lines[i][1];

        // To find the actual line, find the line that is perpendicular
        // to the line through the origin (0, 0) and (rho, theta) and
        // which intersects the line at (rho, theta).

        // convert intersection point coordinates from
        // polar to cartesian coordinates
        double xi = rho * cos(theta);
        double yi = rho * sin(theta);

        // Find the slope of the line through (0,0) and the
        // converted intersection point (rho, theta).
        // This line is perpendicular, so its slope is the inverse of
        // the origin line: m_new = (-1/m).
        double m = -1. / ((yi - 0) / (xi - 0));

        // Since the intersection point (x, y) is known, we can now
        // solve the line equation y = m*x + b for b: b = -(m*x) + y
        double b = -(m * xi) + yi;

        // draw line across the whole image
        Point pt1(0, m * 0 + b);
        Point pt2(image->cols, m * (image->cols) + b);

        // choose line color automatically based on range (0, size_of_list)
        Scalar color = intToRGB(Vec2i(0, lines.size()), i);

        line(*image, pt1, pt2, color, 1);
    }
}


// Checks if a point is on a polar line or not.
// Uses a tolerance to cope with float/double values.
bool pointOnPolarLine (Vec2f point, Vec2f polarLine, double tolerance)
{
    // cartesian coordinates of the point
    double x = point[0];
    double y = point[1];

    // convert polar line pair (rho, theta)
    // to cartesian coordinates (x, y) - the point
    // where the perpendicular line intersects
    // the origin line
    float poX = polarLine[0] * cos(polarLine[1]);
    float poY = polarLine[0] * sin(polarLine[1]);

    // find cartesian equation equal to (rho, theta)
    // (should also be possible directly but no idea how)
    double m = -1. / ((poY - 0) / (poX - 0));
    double b = -(m * poX) + poY;

    // plug in point and check if equation holds (within tolerance)
    // if yes, then the point is on the line.
    double res = m * x + b;

    if (abs(y - res) <= tolerance)
        return true;
    else
        return false;
}


// Create numberRho new lines centered around the current Hough rho cell,
// while keeping the angle theta constant.
// rhoStep denotes the rho resolution of the Hough domain, while
// numRho is the total number of rho values in the matrix.
void clusterCells (int totalCells, float rhoStep, int numRho, Vec2f primaryCell, vector<Vec2f>* clusterLines)
{
    int numberCells = totalCells / 2; // Problem due to rounding uneven values?
    float lRho = primaryCell[0];
    float lTheta = primaryCell[1];

    // start and end (numberCells * rho step) above and below current cell
    // if possible, otherwise cap at bounds
    int rhoclst_start = lRho - (numberCells * rhoStep) < 0 ? 0 : lRho - (numberCells * rhoStep);
    int rhoclst_end = lRho + (numberCells * rhoStep) > numRho * rhoStep ? numRho * rhoStep : lRho + (numberCells * rhoStep);

    // For each accumulator cell (= each accepted line) above the current threshold:
    // Cluster the five rho cells (constant theta) above and below it together with the cell itself
    // This results in a set of a maximum of 11 parallel lines in the cartesian domain, on which component centroids
    // may or may not lie.
    // This is done to catch outliers, i.e. capital letter components whose centroids might not be
    // in line with the small letters in the same word by improving the guess for the rho resolution.
    for(int z = rhoclst_start; z <= rhoclst_end; z+=rhoStep)
        clusterLines->push_back(Vec2f(z, lTheta));
}


// checks if a pixel is purely black
bool isBlack (Vec3b check)
{
    return((check[0] == 0) && (check[1] == 0) && (check[2] == 0));
}

bool isBlack (uchar check)
{
    return (check == 0);
}

// checks if a MBR coordinate is invalid
bool isValidCoord (Vec2i* check)
{
    //return (check != NULL);
    return (!(check[0] == Vec2i(INT_MAX, INT_MAX) || check[1] == Vec2i(-1, -1)));
}

// Two-pass connected-component finding.
// (https://en.wikipedia.org/wiki/Connected-component_labeling)
//
// Expects binary picture (e.g. black layer)
void unionFindComponents(Mat input, vector<ConnectedComponent>* components)
{
    const int rows = input.rows; // shortcuts
    const int cols = input.cols;

    int label = 1; // number of first component
    int** labels; // keeps track of which pixel belongs to which component
    int* pxPerLabel; // number of black pixels in label

    UnionFind* uf; // union-find data structure
    vector<Vec2i> neighborPositions; // (x,y) positions of all black neighbors of current pixel
    vector<int> nb_labels; // contains the labels of the neighbors of a pixel
    set<int> trueLabels; // contains the actually valid labels.

    Vec2i** MBRCoords; // contains min- and max points of each component's MBR



    // initialize labels
    labels = new int*[rows];
    for(int i = 0; i < rows; i++)
        labels[i] = new int[cols];

    // initialize black pixel count per label
    pxPerLabel = new int[rows * cols];

    for(int i = 0; i < (rows * cols); i++)
        pxPerLabel[i] = 0;

    // initialize MBR coordinate array
    MBRCoords = new Vec2i*[rows * cols];
    for(int i = 0; i < (rows * cols); i++)
        MBRCoords[i] = new Vec2i[2];

    // initialize MBR values with dummy values
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
        {
            MBRCoords[(i * cols) + j][0] = Vec2i(INT_MAX, INT_MAX);
            MBRCoords[(i * cols) + j][1] = Vec2i(-1, -1);
        }

    // initialize union-find
    uf = new UnionFind(rows * cols);

    // assign all pixels the "background" label
    for(int i = 0; i < rows; i++)
        for(int j = 0; j < cols; j++)
            labels[i][j] = 0;

    // First pass: Assign labels to pixels. Some of the labels might be
    // equivalent and will be "translated" in the second pass.
    for (int i = 0; i < rows; i++)
        for(int j = 0; j < cols; j++)
            if(isBlack(input.at<uchar>(i, j))) // check only black pixels
            {
                // reset neighbor list
                neighborPositions.clear();

                // 8 EDGE CASES,
                // 1 NORMAL CASE:
                //
                // Consider only pixels "known" pixels, i.e. pixels above the scan line or
                // the pixel's left neighbor - the right neighbor and all pixels below will
                // check above and left of them as well in their iteration
                //
                // top left pixel
                if(i == 0 && j == 0)
                {
                    // always defines a new region
                }

                // top right pixel
                else if(i == 0 && j == (cols - 1))
                {
                    if(isBlack(input.at<uchar>(i, j-1))) neighborPositions.push_back(Vec2i(i, j-1));
                }


                // bottom left pixel
                else if (i == (rows - 1) && j == 0)
                {
                    if(isBlack(input.at<uchar>(i-1, j))) neighborPositions.push_back(Vec2i(i-1, j));
                    if(isBlack(input.at<uchar>(i-1, j+1))) neighborPositions.push_back(Vec2i(i-1, j+1));
                }

                // bottom right pixel
                else if (i == (rows - 1) && j == (cols - 1))
                {
                    if(isBlack(input.at<uchar>(i, j-1))) neighborPositions.push_back(Vec2i(i, j-1));
                    if(isBlack(input.at<uchar>(i-1, j-1))) neighborPositions.push_back(Vec2i(i-1, j-1));
                    if(isBlack(input.at<uchar>(i-1, j))) neighborPositions.push_back(Vec2i(i-1, j));
                }

                // if we were in a corner, the appropriate
                // if-branch would have been triggered already,
                // so inspect only one dimension condition now.
                //
                // upper border
                else if (i == 0)
                {
                    if(isBlack(input.at<uchar>(i, j-1))) neighborPositions.push_back(Vec2i(i, j-1));
                }

                // left border
                else if (j == 0)
                {
                    if(isBlack(input.at<uchar>(i-1, j))) neighborPositions.push_back(Vec2i(i-1, j));
                    if(isBlack(input.at<uchar>(i-1, j+1))) neighborPositions.push_back(Vec2i(i-1, j+1));
                }

                // right border
                else if (j == (cols - 1))
                {
                    if(isBlack(input.at<uchar>(i, j-1))) neighborPositions.push_back(Vec2i(i, j-1));
                    if(isBlack(input.at<uchar>(i-1, j-1))) neighborPositions.push_back(Vec2i(i-1, j-1));
                    if(isBlack(input.at<uchar>(i-1, j))) neighborPositions.push_back(Vec2i(i-1, j));
                }

                // lower border
                else if (i == (rows - 1))
                {
                    if(isBlack(input.at<uchar>(i, j-1))) neighborPositions.push_back(Vec2i(i, j-1));
                    if(isBlack(input.at<uchar>(i-1, j-1))) neighborPositions.push_back(Vec2i(i-1, j-1));
                    if(isBlack(input.at<uchar>(i-1, j))) neighborPositions.push_back(Vec2i(i-1, j));
                    if(isBlack(input.at<uchar>(i-1, j+1))) neighborPositions.push_back(Vec2i(i-1, j+1));
                }

                // Normal case - pixel is somewhere
                // in the image, but not near any border.
                else
                {
                    if(isBlack(input.at<uchar>(i, j-1))) neighborPositions.push_back(Vec2i(i, j-1));
                    if(isBlack(input.at<uchar>(i-1, j-1))) neighborPositions.push_back(Vec2i(i-1, j-1));
                    if(isBlack(input.at<uchar>(i-1, j))) neighborPositions.push_back(Vec2i(i-1, j));
                    if(isBlack(input.at<uchar>(i-1, j+1))) neighborPositions.push_back(Vec2i(i-1, j+1));
                }

                // if there were no black neighbors
                if(neighborPositions.size() == 0)
                {
                    // set MBR to pixel value
                    MBRCoords[label][0] = Vec2i(i, j);
                    MBRCoords[label][1] = Vec2i(i, j);

                    // update label and black pixel count
                    // for this label
                    labels[i][j] = label;
                    pxPerLabel[label] = 1;

                    // next black pixel could be a new label
                    label += 1;
                }

                else
                {
                    // reset list of neighbor labels
                    nb_labels.clear();

                    // begin with the first label ...
                    int minLabel = labels[neighborPositions.front()[0]][neighborPositions.front()[1]];

                    // look at all neighbors' labels
                    for(vector<Vec2i>::iterator nbpIter = neighborPositions.begin(); nbpIter != neighborPositions.end(); nbpIter++)
                    {
                        Vec2i nPos = *nbpIter;

                        // save labels for each neighbor
                        nb_labels.push_back(labels[nPos[0]][nPos[1]]);

                        // find smallest label
                        minLabel = min(minLabel, labels[nPos[0]][nPos[1]]);
                    }

                    // assign smallest label to current pixel
                    // and increase black pixel counter
                    labels[i][j] = minLabel;
                    pxPerLabel[minLabel] += 1;

                    // if the newly labelled pixel changes the MBR
                    // of that label's region, update it
                    MBRCoords[minLabel][0][0] = min(MBRCoords[minLabel][0][0], i);
                    MBRCoords[minLabel][0][1] = min(MBRCoords[minLabel][0][1], j);
                    MBRCoords[minLabel][1][0] = max(MBRCoords[minLabel][1][0], i);
                    MBRCoords[minLabel][1][1] = max(MBRCoords[minLabel][1][1], j);

                    // merge components
                    for (vector<int>::iterator nblIter = nb_labels.begin(); nblIter != nb_labels.end(); nblIter++)
                        uf->merge(minLabel, *nblIter);
                }
            }


    // Second pass: Translate labels (merge equivalent labels) by searching for
    // the set that contains the label and then using it as the "true" label.
    for (int i = 0; i < rows; i++)
        for(int j = 0; j < cols; j++)
        {
            int oldLabel = labels[i][j];
            int newLabel = uf->find(labels[i][j]);

            // skip unused components
            if(!isValidCoord(MBRCoords[oldLabel]))
                continue;

            // if sets were merged ...
            if(oldLabel != newLabel)
            {
                // update MBR bounds if necessary
                MBRCoords[newLabel][0][0] = min(MBRCoords[newLabel][0][0], MBRCoords[oldLabel][0][0]);
                MBRCoords[newLabel][0][1] = min(MBRCoords[newLabel][0][1], MBRCoords[oldLabel][0][1]);
                MBRCoords[newLabel][1][0] = max(MBRCoords[newLabel][1][0], MBRCoords[oldLabel][1][0]);
                MBRCoords[newLabel][1][1] = max(MBRCoords[newLabel][1][1], MBRCoords[oldLabel][1][1]);

                // and add one to the new label's count
                // for every pixel that was changed
                pxPerLabel[newLabel] += 1;
            }

            // update label value and record actually used labels
            labels[i][j] = uf->find(labels[i][j]);
            trueLabels.insert(uf->find(labels[i][j]));
        }

    cout << "SECOND PASS DONE" << "\n\n";


    int numTrueComponents = trueLabels.size();
    cout << "Number of components found: " << numTrueComponents << "\n";


    Mat showMBR;
    cvtColor(input, showMBR, CV_GRAY2RGB);

    // color found components
    for (int i = 0; i < rows; i++)
        for(int j = 0; j < cols; j++)
        {
            if(labels[i][j] != 0)
            {
                Vec3b color = intToRGB(Vec2i(0, numTrueComponents), labels[i][j]);
                showMBR.at<Vec3b>(i, j) = color;
            }

            else
                showMBR.at<Vec3b>(i, j) = showMBR.at<Vec3b>(i, j);
        }



    // retrieve MBRs and show them
    for(set<int>::iterator iter = trueLabels.begin(); iter != trueLabels.end(); iter++)
    {
        // skip coordinates of invalid sets
        if(!isValidCoord(MBRCoords[*iter]))
            continue;

        // create connected component
        // and store it in vector
        components->push_back(*(new ConnectedComponent(MBRCoords[*iter][0], MBRCoords[*iter][1], pxPerLabel[*iter])));

        // rectangle works with (col,row) for some
        // reason, so swap coordinates
        Point min = Vec2i(MBRCoords[*iter][0][1], MBRCoords[*iter][0][0]);
        Point max = Vec2i(MBRCoords[*iter][1][1], MBRCoords[*iter][1][0]);

        // draw MBR for this component
        rectangle(showMBR, min, max, Scalar(0, 0, 255), 1, 8, 0);
        //putText(comps, to_string(num), Point(min*0.5), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,0,255), 1, 8, false);
    }

    //for(vector<ConnectedComponent>::iterator iter = components->begin(); iter != components->end(); iter++)
    //    cout << *iter;

    // cleanup
    delete [] labels;
    delete [] pxPerLabel;
    delete [] MBRCoords;
    delete uf;

    // show result
    namedWindow("Components", WINDOW_AUTOSIZE);
    imshow("Components", showMBR);
}

// Dismiss any components that have an area ratio
// less than 1:ratio or larger than ratio:1 because they
// are likely to not be characters.
void ratioFilter(Mat input, vector<ConnectedComponent>* components, int ratio)
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


// Performs collinear grouping of potential characters
// via Hough (line) transformation on the MBR centroids of all components
//
// Explained in "A Robust Algorithm for Text String Separation from
// Mixed Text/Graphics Images" by L. Fletcher and R. Kasturi
void collinearGrouping (Mat input, vector<ConnectedComponent>* comps)
{
    int rows = input.rows;
    int cols = input.cols;
    double avgheight = 0;

    // create new matrix for performing
    // Hough transform on centroids only
    Mat hough = Mat(rows, cols, CV_8U);

    // set entire matrix to background color (= black, because hough expects edges in white)
    for(int i = 0; i < rows; i++)
        for(int j = 0; j < cols; j++)
            hough.at<uchar>(i, j) = 0;

    // extract centroids of connected components
    for (vector<ConnectedComponent>::iterator iter = comps->begin(); iter != comps->end(); iter++)
    {
        ConnectedComponent curr = *iter;

        hough.at<uchar>(curr.centroid[0], curr.centroid[1]) = 255; // mark centroid as white in Hough matrix
        avgheight += curr.mbr_max[0] - curr.mbr_min[0]; // cumulative height of all components
    }

    //namedWindow("CENTROIDS", WINDOW_AUTOSIZE);
    //imshow("CENTROIDS", hough);

    // output matrix for showing found lines
    Mat showHough;
    cvtColor(input, showHough, CV_GRAY2RGB);

    // calculate average height of all components
    avgheight /= comps->size();

    float rho = 0.2 * avgheight; // set rho step (the polar line distance value resolution) to initial guess
    float theta = 0.0174533; // set theta (vector angle) resolution in radians
    int threshold = 20; // values in Hough accumulator have to exceed this value to be accepted
    int numAngle = floor((M_PI / theta) + 0.5); // number of theta steps; round up on 0.5
    int numRho = cvRound(((cols + rows) * 2 + 1) / rho); // number of rho steps
    int counter = 0; // when this becomes 2, the algorithm stops.

    int* accumulator = new int[(numAngle+2) * (numRho+2)]; // accumulator matrix being used in all hough transformations hereafter
    memset(accumulator, 0, (sizeof(int) * (numAngle+2) * (numRho+2)));

    vector<Vec2f> lines; // will contain all found lines

    // Do multiple hough transforms using the same accumulator while limiting
    // the angle of the lines to 0° - 5°, 85° - 95° and 175° - 180° respectively
    // to find all vertically or horizontally aligned components
    //HoughLinesCustom(hough, rho, theta, threshold, lines, numRho * numAngle, 0.0, 0.0872665, accumulator);
    //HoughLinesCustom(hough, rho, theta, threshold, lines, numRho * numAngle, 1.48353, 1.65806, accumulator);
    //HoughLinesCustom(hough, rho, theta, threshold, lines, numRho * numAngle, 3.05433, 3.14159, accumulator);

    HoughLines(hough, lines, rho, theta, 0, 0, threshold, 0.0, 0.0872665);
    HoughLines(hough, lines, rho, theta, 0, 0, threshold, 1.48353, 1.65806);
    HoughLines(hough, lines, rho, theta, 0, 0, threshold, 3.05433, 3.14159);


    vector<Vec2f> clusterLines; // lines parallel to the initial line
    vector<ConnectedComponent> cluster; // components that lie on clusterLines
    avgheight = 0.; // Reset average height

    // Matrix for showing component's clustering
    Mat clusters = hough.clone();
    cvtColor(clusters, clusters, CV_GRAY2RGB);
    drawLines(lines, &hough, Scalar(0, 255, 0));

    int finalCluster = 0;
    // text/graphic segmentation loop
    while (counter < 2)
    {
        while (threshold > 2)
        {
            // PAPER STEPS 4-10: for all INITIAL LINES
            for (vector<Vec2f>::iterator houghLine = lines.begin(); houghLine != lines.end(); houghLine++)
            {
                // 5) Cluster 11 rho cells (including the primary cell) around the primary cell
                clusterCells (10, rho, numRho, *houghLine, &clusterLines);

                // show lines in red
                //drawLines(lines, &clusters, Scalar(0, 0, 255));

                // find components to cluster together by calculating rho and theta values
                // from the centroid coordinates and comparing them to the cluster line values.
                // Then, calculate their average MBR height.
                for(vector<Vec2f>::iterator clLine = clusterLines.begin(); clLine != clusterLines.end(); clLine++)
                {
                    // check if components lie on cluster line
                    // if yes, add them to the cluster
                    for(vector<ConnectedComponent>::iterator component = comps->begin(); component != comps->end(); component++)
                        if(pointOnPolarLine((*component).centroid, *clLine, 1.))
                        {
                            cluster.push_back(*component);
                            avgheight += (*component).mbr_max[0] - (*component).mbr_min[0];
                        }
                }

                // initial clustering only needed for average height calculation
                clusterLines.clear();

                // 7) Compute new clustering factor
                // (= amount of rho cells to cluster)
                cluster.size() == 0 ? avgheight = avgheight : avgheight /= cluster.size();
                avgheight /= cluster.size();
                int factor = avgheight / rho;

                // 8) Re-cluster using the new factor
                cluster.clear();
                clusterCells(factor, rho, numRho, *houghLine, &clusterLines);

                // find all components that lie in close proximity (= "on") a cluster line
                for(vector<Vec2f>::iterator clLine = clusterLines.begin(); clLine != clusterLines.end(); clLine++)
                {
                    for(vector<ConnectedComponent>::iterator component = comps->begin(); component != comps->end(); component++)
                    {
                        if(pointOnPolarLine((*component).centroid, *clLine, 1.))
                            cluster.push_back(*component);
                    }
                }

                // clusters are formed, so the cluster lines are no longer needed
                clusterLines.clear();

                // 9) Separate strings from graphics in the cluster and delete them.
                for(vector<ConnectedComponent>::iterator clusComp = cluster.begin(); clusComp != cluster.end(); clusComp++)
                {
                    // debug: remove centroid from cluster matrix
                    Vec2f cen = (*clusComp).centroid;

                    //rectangle(clusters, Point(cen[1], cen[0]), Point(cen[1], cen[0]), Scalar(0, 0, 0));
                    Vec3i color = intToRGB(Vec2i(0, cluster.size()), clusComp - cluster.begin());
                    //circle(clusters, Point(cen[1], cen[0]), 10, color, 1, 8, 0);
                    putText(clusters, to_string(finalCluster), Point(cen[1], cen[0]), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0,255,0), 1, 8, false);

                    // 10) Refresh the accumulator by deleting the contributions made by
                    // each deleted component.
                    /*for(int n = 0; n < numAngle; n++ )
                    {
                        // calculate line set cells
                        // for the lines passing through the centroid
                        int r = cvRound( cen[0] * cos(n * theta) + cen[1] * sin(n * theta));
                        r += (numRho - 1) / 2;

                        // remove contributed value from cell
                        accumulator[(n+1) * (numRho+2) + r+1]--;
                    }*/
                }

                finalCluster++;

                // clustering for this hough line is done
                cluster.clear();
            }

            // decrement accumulator threshold
            cout << "LINESNOW: " << lines.size() << " with THRESHOLD: " << threshold << "\n";
            threshold--;

            // add all accumulator cells that can be
            // admitted because they now meet the new, lower threshold
            /*lines.clear();

            for(int r = 0; r < numRho; r++ )
            {
                for(int n = 0; n < numAngle; n++ )
                {
                    int base = (n+1) * (numRho+2) + r+1;
                    if( accumulator[base] == threshold && // match threshold exactly since we only want to add lines that aren't added already
                            accumulator[base] > accumulator[base - 1] &&
                            accumulator[base] >= accumulator[base + 1] &&
                            accumulator[base] > accumulator[base - numRho - 2] &&
                            accumulator[base] >= accumulator[base + numRho + 2] )
                    {
                        double scale = 1./(numRho+2);
                        LinePolar line;
                        int idx = accumulator[base];
                        int n = cvFloor(idx*scale) - 1;
                        int r = idx - (n+1)*(numRho+2) - 1;
                        line.rho = (r - (numRho - 1)*0.5f) * rho;
                        line.angle = n * theta;
                        line.thresh = threshold;
                        lines.push_back(line);
                    }
                }
            }*/

            // no way yet to simply grab lines with a lower threshold
            // IDEA: custom hough copies its accumulator into supplied zero matrix (adds values)
            lines.clear();
            HoughLines(hough, lines, rho, theta, 0, 0, threshold, 0.0, 0.0872665);
            HoughLines(hough, lines, rho, theta, 0, 0, threshold, 1.48353, 1.65806);
            HoughLines(hough, lines, rho, theta, 0, 0, threshold, 3.05433, 3.14159);
        }

        if(counter == 0)
        {
            // calculate hough domain for lines with angles [0°, 180°]
            //HoughLinesCustom(hough, rho, theta, threshold, lines, numRho * numAngle, 0., 3.14159, accumulator);
            HoughLines(hough, lines, rho, theta, 0, 0, threshold, 0., 3.14159);
            cout << "\n --------- LAST ITERATION. ---------- \n";

            // reset threshold
            threshold = 20;

            // do one last iteration
            counter++;
        }

        else
            counter++;
    }

    namedWindow("CLUSTER", WINDOW_AUTOSIZE);
    imshow("CLUSTER", clusters);

    // show hough lines on input image
    //drawLines(lines, &showHough, Scalar(0, 255, 0));

    // cleanup
    delete [] accumulator;


    // show result
    namedWindow("HOUGH", WINDOW_AUTOSIZE);
    imshow("HOUGH", showHough);

    waitKey(0);
}

/**
 * @brief Extract a black layer from an image. If a pixel
 * is below the supplied tresholds for each channel, the
 * pixel is assumed to be black.
 * @param input The input image in matrix form.
 * @param output The black layer in matrix form.
 */
void getBlackLayer(Vec3b thresholds, Mat input, Mat* output)
{
    for(int i = 0; i < input.cols; i++)
    {
        for(int j = 0; j < input.rows; j++)
        {
            Vec3b currentPixel = input.at<Vec3b>(j, i);

            // check thresholds (Vec3b is BGR!)
            if(currentPixel[0] <= thresholds[0] &&
                    currentPixel[1] <= thresholds[1] &&
                    currentPixel[2] <= thresholds[2])
            {
                // pixel below all thresholds: make it black
                output->at<uchar>(j, i) = 0;
            }

            // else: make pixel white
            else
                output->at<uchar>(j, i) = 255;
        }
    }

    namedWindow("black layer", WINDOW_AUTOSIZE);
    imshow("black layer", *output);
}


int main (int argc, char** argv)
{
    Mat original, *output;
    vector<ConnectedComponent> components;

    //original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/CV_sample_schwer_2.png", CV_LOAD_IMAGE_COLOR);
    //original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/peter.png", CV_LOAD_IMAGE_COLOR);
    original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/jointest.png", CV_LOAD_IMAGE_COLOR);

    // output matrix will be binary (use uchar instead of Vec3b)
    output = new Mat(original.rows, original.cols, CV_8U);

    // BGR format
    // Thresholds need testing with unscaled images to avoid
    // artifacts artificially increasing the needed threshold
    //Vec3b thresholds = Vec3b(160, 160, 160);
    Vec3b thresholds = Vec3b(180, 180, 180);

    getBlackLayer(thresholds, original, output);
    unionFindComponents(*output, &components);
    ratioFilter(*output, &components, 10);
    collinearGrouping(*output, &components);
}
