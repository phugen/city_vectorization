/**
  * Implementation of a text extraction algorithm for images,
  * as described by L. Fletcher and R. Kasturi in their publication
  * "A Robust Algorithm for Text String Separation from Mixed Text/Graphics Images".
  *
  * Author: Philipp Hugenroth, 2015
  */

#define _USE_MATH_DEFINES

#include "include/collineargrouping.hpp"
#include <cmath>
#include <iostream>

#include "include/auxiliary.hpp"
#include "include/colorconversions.hpp"
#include "include/customhoughtransform.hpp"
#include "include/areafilter.hpp"
#include "include/stringheuristics.hpp"

using namespace std;
using namespace cv;

// A < B, if its A's distance to the associated hough line
// is smaller than B's distance to the same (!) hough line.
bool compareByLineDistance (ConnectedComponent a, ConnectedComponent b)
{
    return (distanceFromPolarLine(a.centroid, a.houghLine) <
            (distanceFromPolarLine(b.centroid, b.houghLine)));
}

// Performs collinear grouping and deletion of potential characters
// via Hough transformation on the MBR centroids of all components.
void collinearGrouping (Mat input, vector<ConnectedComponent>* comps)
{
    int rows = input.rows;
    int cols = input.cols;
    double avgheight = 0;

    // create new matrix for performing
    // Hough transform on centroids only
    Mat hough_UC = Mat(rows, cols, CV_8U);

    // set entire matrix to background color (= black, because hough expects edges to be white)
    for(int i = 0; i < rows; i++)
        for(int j = 0; j < cols; j++)
            hough_UC.at<uchar>(i, j) = 0;

    // extract centroids of connected components
    for (vector<ConnectedComponent>::iterator iter = comps->begin(); iter != comps->end(); iter++)
    {
        ConnectedComponent curr = *iter;

        hough_UC.at<uchar>(curr.centroid[0], curr.centroid[1]) = 255; // mark centroid as white in Hough matrix
        avgheight += curr.mbr_max[0] - curr.mbr_min[0]; // cumulative height of all components
    }

    namedWindow("CENTROIDS", WINDOW_AUTOSIZE);
    imshow("CENTROIDS", hough_UC);

    // output matrix for showing found lines
    Mat showHough;
    cvtColor(input, showHough, CV_GRAY2RGB);

    // matrix that indicates which components are
    // part of the current cluster
    Mat clusterMat_V3;
    cvtColor(hough_UC, clusterMat_V3, CV_GRAY2RGB);

    // output matrix
    Mat erased = input.clone();

    // calculate average height of all components
    avgheight /= comps->size();

    float guess_factor = 0.2 * avgheight; // Initial guess for the rho resolution
    float rho = guess_factor; // set rho step (the polar line distance value resolution) to initial guess
    float theta = 0.0174533; // set theta (vector angle) resolution in radians
    int threshold = 20; // values in Hough accumulator have to exceed this value to be accepted
    int numAngle = floor((M_PI / theta) + 0.5); // number of theta steps; round up on 0.5
    int numRho = cvRound(((cols + rows) * 2 + 1) / rho); // number of rho steps
    int counter = 0; // when this becomes 2, the algorithm stops.

    int* accumulator = new int[(numAngle+2) * (numRho+2)]; // accumulator matrix to pass to HoughLinesCustom to retain accum information
    memset(accumulator, 0, (sizeof(int) * (numAngle+2) * (numRho+2)));

    vector<Vec2f> lines (0, 0); // will contain all found lines
    vector<Vec2i> allContributions (0, 0); // stores (numrho, numangle) tuples for a line in "lines "at the same position


    // Do multiple hough transforms using the same accumulator while limiting
    // the angle of the lines to 0° - 5°, 85° - 95° and 175° - 180° respectively
    // to find all vertically or horizontally aligned components
    HoughLinesCustom(hough_UC, rho, theta, 0.0, 0.0872665, accumulator);
    HoughLinesExtract (accumulator, numRho, numAngle, rho, theta, 0.0, threshold, &lines, &allContributions, THRESH_GT);

    HoughLinesCustom(hough_UC, rho, theta, 1.48353, 1.65806, accumulator);
    HoughLinesExtract (accumulator, numRho, numAngle, rho, theta, 1.48353, threshold, &lines, &allContributions, THRESH_GT);

    cout << "LINESNOW: " << lines.size() << " with THRESHOLD: " << threshold << "\n";


    // show hough lines
    drawLines(lines, &showHough, Scalar(0, 255, 0));
    //drawLines(lines, &hough_8U, Scalar(255, 0, 0));

    vector<Vec2f> clusterLines; // lines parallel to the initial line
    vector<ConnectedComponent> cluster; // components that lie on clusterLines
    vector<Vec2f> clusterContributions; // all accumulator positions of lines that lead to the clustering, TBD

    avgheight = 0.; // Reset average height
    int clusterNo = 0;

    // text/graphic segmentation loop
    while (counter < 2)
    {
        while (threshold > 6)
        {
            // PAPER STEPS 4-10: for all INITIAL LINES
            for (vector<Vec2f>::iterator houghLine = lines.begin(); houghLine != lines.end(); houghLine++)
            {
                // 5) Cluster 11 rho cells (including the primary cell) around the primary cell
                clusterCells (10, rho, numRho, *houghLine, &clusterLines);

                // show cluster lines in green
                //drawLines(clusterLines, &showHough, Scalar(0, 255, 0));

                // Check for each component if it lies on a cluster line.
                // If yes, add them to the current cluster.
                //
                // Simultaneously, calculate the average MBR height
                // of all components in the cluster in order to refine
                // the rho resolution guess.
                for(vector<Vec2f>::iterator clLine = clusterLines.begin(); clLine != clusterLines.end(); clLine++)
                    for(vector<ConnectedComponent>::iterator component = comps->begin(); component != comps->end(); component++)
                        if(pointOnPolarLine((*component).centroid, *clLine, 2.))
                        {
                            cluster.push_back(*component);
                            avgheight += (*component).mbr_max[0] - (*component).mbr_min[0];
                        }

                // 7) Compute new clustering factor
                // (= amount of rho cells to cluster)
                cluster.size() == 0 ? avgheight = avgheight : avgheight /= cluster.size(); // avoid division by zero
                int factor = avgheight / rho;

                // 8) Reset cluster and re-cluster using the new factor
                cluster.clear();
                clusterCells(factor, rho, numRho, *houghLine, &clusterLines);

                // find all components that lie in close proximity (= "on") a cluster line
                for(vector<Vec2f>::iterator clLine = clusterLines.begin(); clLine != clusterLines.end(); clLine++)
                    for(vector<ConnectedComponent>::iterator component = comps->begin(); component != comps->end(); component++)
                        if(pointOnPolarLine((*component).centroid, *clLine, 2.))
                        {
                            // mark all cluster centroids which are in this cluster as blue
                            clusterMat_V3.at<Vec3b>((*component).centroid[0], (*component).centroid[1]) = Vec3b(255, 0, 0);

                            // associate current Hough line with the component
                            (*component).houghLine = *houghLine;

                            // add this component to the refined cluster
                            cluster.push_back(*component);
                        }

                // apply area ratio filter to eliminate large components from the cluster
                // whose centroids are coincidentally on a string's hough line
                areaFilter(&cluster, 5);

                // sort components in this cluster by their distance to the original hough line
                sort(cluster.begin(), cluster.end(), compareByLineDistance);

                // Refine the cluster further by using heuristics such as the inter-component gap.
                for(vector<ConnectedComponent>::iterator clusComp = cluster.begin(); clusComp != cluster.end(); clusComp++)
                {
                    // calculate average local height for this component
                    double avgHeight = localAvgHeight(cluster, clusComp - cluster.begin());

                    // Intercomponent gap must be <= average local height
                    // for the component to stay in the cluster
                    if(clusComp != cluster.end() || cluster.size() == 1)
                        if(distanceBetweenPoints((*clusComp).centroid, (*(clusComp+1)).centroid) * 2.5 > avgHeight)
                            cluster.erase(clusComp); // remove the unrelated component
                }

                // 9) Separate strings from graphics in the cluster and delete them.
                for(vector<ConnectedComponent>::iterator clusComp = cluster.begin(); clusComp != cluster.end(); clusComp++)
                {
                    // Strings need to have at least two components
                    if(cluster.size() >= 2)
                        eraseComponentPixels(*clusComp, &erased);

                    // debug: delete deleted centroids
                    //clusterMat_V3.at<Vec3b>((*clusComp).centroid[0], (*clusComp).centroid[1]) = Vec3b(0, 0, 0);

                    // delete those values from the accumulator which were contributed
                    // to it by components in the deleted cluster
                    //deleteLineContributions (accumulator, numAngle, allContributions);
                }

                //namedWindow("CURRENT CLUSTER", WINDOW_AUTOSIZE);
                //imshow("CURRENT CLUSTER", clusterMat_V3);


                //cout << "CLUSTER SIZE: " << cluster.size() << "\n\n";

                // clustering for this hough line is done
                cluster.clear();

                // next cluster
                clusterNo++;
            }

            //waitKey(0);

            // decrement accumulator threshold
            threshold--;

            // if this is the first pass (= nearly horizontal and vertical line detection)
            if(counter == 0)
            {
                // extract all lines that now exactly meet the threshold
                // (they were lower than the previous threshold, and can now be admitted)
                HoughLinesExtract (accumulator, numRho, numAngle, rho, theta, 0., threshold, &lines, &allContributions, THRESH_EQ);
                HoughLinesExtract (accumulator, numRho, numAngle, rho, theta, 1.48353, threshold, &lines, &allContributions, THRESH_EQ);

                cout << "LINESNOW: " << lines.size() << " with THRESHOLD: " << threshold << "\n";
            }

            // if this is the second pass (0 - 180°)
            else
            {
                HoughLinesExtract (accumulator, numRho, numAngle, rho, theta, 0., threshold, &lines, &allContributions, THRESH_EQ);

                cout << "LINESNOW: " << lines.size() << " with THRESHOLD: " << threshold << "\n";
            }
        }

        // first pass is done
        if(counter == 0)
        {
            cout << "\n --------- SECOND PASS. ---------- \n";

            // show post-first pass (vertical/horizontal) hough lines in blue
            drawLines(lines, &showHough, Scalar(255, 0, 0));
            //drawLines(lines, &hough_8U, Scalar(255, 0, 0));

            // debug: map hough space to image
            //mapHoughToImage (rows, cols, theta, rho, numAngle, numRho, accumulator);

            // clear extracted lines and accumulator
            lines.clear();
            memset(accumulator, 0, (sizeof(int) * (numAngle+2) * (numRho+2)));

            // reset threshold
            threshold = 20;

            // calculate hough domain for lines with angles [0°, 180°]
            HoughLinesCustom(hough_UC, rho, theta, 0., 3.14159, accumulator);
            HoughLinesExtract (accumulator, numRho, numAngle, rho, theta, 0., threshold, &lines, &allContributions, THRESH_GT);

            cout << "LINESNOW: " << lines.size() << " with THRESHOLD: " << threshold << "\n";


            // do one last iteration
            counter++;
        }

        // second pass done, end algorithm
        else
        {
            // show all 0 - 180° hough lines in red
            drawLines(lines, &showHough, Scalar(0, 0, 255));
            //drawLines(lines, &hough_8U, Scalar(255, 0, 0));

            counter++;
        }
    }

    // show hough lines overlayed on image
    namedWindow("HOUGH+IMAGE", WINDOW_AUTOSIZE);
    imshow("HOUGH+IMAGE", showHough);

    // show hough lines on component centroids
    namedWindow("HOUGH+CENTROIDS", WINDOW_AUTOSIZE);
    imshow("HOUGH+CENTROIDS", hough_UC);

    // show result
    namedWindow("WITHOUT TEXT", WINDOW_AUTOSIZE);
    imshow("WITHOUT TEXT", erased);

    // show clustering
    //imshow("CLUSTER", clusters);

    // cleanup
    delete [] accumulator;

    waitKey(0);
}
