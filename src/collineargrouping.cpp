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

    namedWindow("CENTROIDS", WINDOW_AUTOSIZE);
    imshow("CENTROIDS", hough);

    // output matrix for showing found lines
    Mat showHough;
    cvtColor(input, showHough, CV_GRAY2RGB);

    // calculate average height of all components
    avgheight /= comps->size();

    float guess_factor = 0.2 * avgheight;
    float rho = guess_factor; // set rho step (the polar line distance value resolution) to initial guess
    float theta = 0.0174533; // set theta (vector angle) resolution in radians
    int threshold = 20; // values in Hough accumulator have to exceed this value to be accepted
    int numAngle = floor((M_PI / theta) + 0.5); // number of theta steps; round up on 0.5
    int numRho = cvRound(((cols + rows) * 2 + 1) / rho); // number of rho steps
    int counter = 0; // when this becomes 2, the algorithm stops.

    int* accumulator = new int[(numAngle+2) * (numRho+2)]; // accumulator matrix being used in all hough transformations hereafter
    memset(accumulator, 0, (sizeof(int) * (numAngle+2) * (numRho+2)));

    vector<Vec2f> tempLines (0,0); // for temporary storage after a Hough transformation.
    vector<Vec2f> lines (0, 0); // will contain all found lines

    // Do multiple hough transforms using the same accumulator while limiting
    // the angle of the lines to 0° - 5°, 85° - 95° and 175° - 180° respectively
    // to find all vertically or horizontally aligned components
    HoughLines(hough, tempLines, rho, theta, 0, 0, threshold, 0.0, 0.0872665);
    lines.insert(lines.end(), tempLines.begin(), tempLines.end());

    HoughLines(hough, tempLines, rho, theta, 0, 0, threshold, 1.48353, 1.65806);
    lines.insert(lines.end(), tempLines.begin(), tempLines.end());

    HoughLines(hough, tempLines, rho, theta, 0, 0, threshold, 3.05433, 3.14159);
    lines.insert(lines.end(), tempLines.begin(), tempLines.end());
    cout << "LINESNOW: " << lines.size() << " with THRESHOLD: " << threshold << "\n";

    // Matrix for showing component's clustering
    Mat clusters = hough.clone();
    cvtColor(clusters, clusters, CV_GRAY2RGB);

    // show hough lines
    drawLines(lines, &showHough, Scalar(0, 255, 0));


    vector<Vec2f> clusterLines; // lines parallel to the initial line
    vector<ConnectedComponent> cluster; // components that lie on clusterLines
    vector<vector<ConnectedComponent>> cluster_id (1); // keeps track of which component is part of what cluster

    avgheight = 0.; // Reset average height
    int clusterNo = 0;

    // text/graphic segmentation loop
    while (counter < 2)
    {
        while (threshold > 2)
        {
            // show hough lines in red
            drawLines(lines, &showHough, Scalar(0, 0, 255));

            // PAPER STEPS 4-10: for all INITIAL LINES
            for (vector<Vec2f>::iterator houghLine = lines.begin(); houghLine != lines.end(); houghLine++)
            {
                // add new cluster "label"
                cluster_id.push_back(vector<ConnectedComponent>());

                // 5) Cluster 11 rho cells (including the primary cell) around the primary cell
                clusterCells (10, rho, numRho, *houghLine, &clusterLines);

                // show cluster lines in blue
                //drawLines(clusterLines, &showHough, Scalar(255, 0, 0));

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

                // 7) Compute new clustering factor
                // (= amount of rho cells to cluster)
                cluster.size() == 0 ? avgheight = avgheight : avgheight /= cluster.size(); // avoid division by zero
                int factor = avgheight / rho;

                // 8) Re-cluster using the new factor
                cluster.clear();
                clusterCells(factor, rho, numRho, *houghLine, &clusterLines);

                //cout << "FACTOR: " << guess_factor << " VS " << factor << "\n";

                // find all components that lie in close proximity (= "on") a cluster line
                for(vector<Vec2f>::iterator clLine = clusterLines.begin(); clLine != clusterLines.end(); clLine++)
                {
                    for(vector<ConnectedComponent>::iterator component = comps->begin(); component != comps->end(); component++)
                    {
                        if(pointOnPolarLine((*component).centroid, *clLine, 1.))
                        {
                            cluster.push_back(*component);
                            cluster_id.at(clusterNo).push_back(*component); // add to list of comps that are in this cluster
                        }
                    }
                }

                // 9) Separate strings from graphics in the cluster and delete them.
                for(vector<ConnectedComponent>::iterator clusComp = cluster.begin(); clusComp != cluster.end(); clusComp++)
                {

                    // DELETE HERE
                    //eraseComponentPixels(*clusComp, &hough);


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

                //cout << "CLUSTER SIZE: " << cluster.size() << "\n\n";

                // clustering for this hough line is done
                cluster.clear();

                // next cluster
                clusterNo++;

                //namedWindow("CLUSTER", WINDOW_AUTOSIZE);
                //imshow("CLUSTER", clusters);

                //waitKey(0);
            }

            // decrement accumulator threshold
            threshold--;

            // add all accumulator cells that can be
            // admitted because they now meet the new, lower threshold
            // --------------------------------------------------------

            // if this is the first pass
            if(counter == 0)
            {
                // no way yet to simply grab lines with a lower threshold
                // IDEA: custom hough adds its accumulator to supplied matrix
                lines.clear();

                HoughLines(hough, tempLines, rho, theta, 0, 0, threshold, 0.0, 0.0872665);
                lines.insert(lines.end(), tempLines.begin(), tempLines.end());

                HoughLines(hough, tempLines, rho, theta, 0, 0, threshold, 1.48353, 1.65806);
                lines.insert(lines.end(), tempLines.begin(), tempLines.end());

                HoughLines(hough, tempLines, rho, theta, 0, 0, threshold, 3.05433, 3.14159);
                lines.insert(lines.end(), tempLines.begin(), tempLines.end());

                cout << "LINESNOW: " << lines.size() << " with THRESHOLD: " << threshold << "\n";
            }
        }

        // if this is the first pass
        if(counter == 0)
        {
            lines.clear();

            // reset threshold
            threshold = 20;

            // calculate hough domain for lines with angles [0°, 180°]
            cout << "\n --------- LAST ITERATION. ---------- \n";
            HoughLines(hough, tempLines, rho, theta, 0, 0, threshold, 0., 3.14159);
            lines.insert(lines.end(), tempLines.begin(), tempLines.end());
            cout << "LINESNOW: " << lines.size() << " with THRESHOLD: " << threshold << "\n";

            // do one last iteration
            counter++;
        }
    }



    int ct = 0;
    // color-code component allegiance
    for(auto iter = comps->begin(); iter != comps->end(); iter++)
        for(auto cluster = cluster_id.begin(); cluster != cluster_id.end(); cluster++)
        {
            //cout << ct << ": ";

            for(auto incluster = (*cluster).begin(); incluster != (*cluster).end(); incluster++)
            {
                Vec2f cen = (*incluster).centroid;
                //cout << (*incluster).centroid << ", ";

                //rectangle(clusters, Point(cen[1], cen[0]), Point(cen[1], cen[0]), Scalar(0, 0, 0));
                Vec3i color = intToRGB(Vec2i(0, comps->size()), cluster - cluster_id.end());
                circle(clusters, Point(cen[1], cen[0]), 10, color, 1, 8, 0);
                //putText(clusters, to_string(finalCluster), Point(cen[1], cen[0]), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0,255,0), 1, 8, false);
            }

            //cout << "\n";
            ct++;
        }


    // show hough lines
    namedWindow("HOUGH", WINDOW_AUTOSIZE);
    imshow("HOUGH", showHough);

    // show clustering
    imshow("CLUSTER", clusters);

    // cleanup
    delete [] accumulator;

    waitKey(0);
}
