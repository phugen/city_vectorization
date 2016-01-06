#define _USE_MATH_DEFINES

#include "include/collineargrouping.hpp"
#include <cmath>
#include <iostream>

#include "include/auxiliary.hpp"
#include "include/colorconversions.hpp"

using namespace std;
using namespace cv;

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
