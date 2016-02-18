/**
  * Implementation of a text extraction algorithm for images,
  * as described by L. Fletcher and R. Kasturi in their publication
  * "A Robust Algorithm for Text String Separation from Mixed Text/Graphics Images".
  *
  * Author: Phugen
  */

#define _USE_MATH_DEFINES

#include "include/collineargrouping.hpp"
#include <cmath>
#include <iostream>
#include <map>

#include "include/auxiliary.hpp"
#include "include/colorconversions.hpp"
#include "include/customhoughtransform.hpp"
#include "include/areafilter.hpp"
#include "include/collinearstring.hpp"

using namespace std;
using namespace cv;

#define DEBUG


void printAccumValues (int* accumulator, int numAngle, int numRho)
{
    for(int i = 0; i < numAngle; i++)
    {
        for(int j = 0; j < numRho; j++)
        {
            cout << accumulator[i * numRho + j];
        }

        cout << "\n";
    }
    cout << "\n\n";
}


// Compare two components that lie on the same Hough line
// by their distance along the line, i.e. their centroid's
// X coordinate.
bool compareByLineDistance (ConnectedComponent a, ConnectedComponent b)
{
    return (a.centroid[1]) < (b.centroid[1]);
}

// Performs collinear grouping and deletion of potential characters
// via Hough transformation on the MBR centroids of all components.
void collinearGrouping (Mat input, vector<ConnectedComponent>* comps)
{
    // No components passed the filters - no work left to do.
    if(comps->size() == 0)
        return;

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
    cvtColor(input, clusterMat_V3, CV_GRAY2RGB);

    // output matrix
    Mat erased = input.clone();

    // calculate average height of all components
    avgheight /= comps->size();

    float guess_factor = 0.2 * avgheight; // Initial guess for the rho resolution: average height of components
    guess_factor > 0 ? guess_factor = guess_factor : guess_factor = 1;

    float rho = guess_factor; // set rho step (the polar line distance value resolution) to initial guess
    float theta = 0.0174533; // set theta (vector angle) resolution in radians (1 degree).
    int threshold = 20; // values in Hough accumulator have to exceed this value to be accepted
    int numAngle = floor((M_PI / theta) + 0.5); // number of theta steps; round up on 0.5
    int numRho = cvRound(((cols + rows) * 2 + 1) / rho); // number of rho steps
    int counter = 0; // when this becomes 2, the algorithm stops.

    int* accumulator = new int[(numAngle+2) * (numRho+2)]; // accumulator matrix to pass to HoughLinesCustom to retain accum information
    memset(accumulator, 0, (sizeof(int) * (numAngle+2) * (numRho+2))); // initialize accumulator with zero values

    vector<Vec3f> lines; // will contain all found lines
    map<Vec2i, vector<int>, Vec2iCompare> contributions; // stores accumulator contribution positions for all input points

    // Do multiple hough transforms using the same accumulator while limiting
    // the angle of the lines to 0° - 5°, 85° - 95° and 175° - 180° respectively
    // to find all vertically or horizontally aligned components
    HoughLinesCustom(hough_UC, rho, theta, 0.0, 0.0872665, accumulator, &contributions);
    HoughLinesExtract(accumulator, numRho, numAngle, rho, theta, 0.0, threshold, &lines, THRESH_GT);

    HoughLinesCustom(hough_UC, rho, theta, 1.48353, 1.65806, accumulator, &contributions);
    HoughLinesExtract(accumulator, numRho, numAngle, rho, theta, 1.48353, threshold, &lines, THRESH_GT);

    HoughLinesCustom(hough_UC, rho, theta, 3.05433, 3.14159, accumulator, &contributions);
    HoughLinesExtract(accumulator, numRho, numAngle, rho, theta, 3.05433, threshold, &lines, THRESH_GT);

    cout << "LINESNOW: " << lines.size() << " with THRESHOLD: " << threshold << "\n";


    // show hough lines
    //drawLines(lines, &showHough, Scalar(0, 255, 0));
    //drawLines(lines, &hough_8U, Scalar(255, 0, 0));

    vector<Vec3f> clustered_cells; // accumulator cell positions of cells in the cluster
    vector<ConnectedComponent> cluster; // components that lie on clusterLines
    vector<CollinearString> collinearStrings; // contains meta information gained from clusters

    avgheight = 0.; // Reset average height
    int clusterNo = 0;

    // text/graphic segmentation loop
    while (counter < 2)
    {
        while (threshold > 2)
        {
            // PAPER STEPS 4-10: for all INITIAL LINES
            for (auto houghLine = lines.begin(); houghLine != lines.end(); houghLine++)
            {
                // 5) Cluster 11 rho cells (including the primary cell) around the primary cell
                clusterCells (10, rho, numRho, numAngle, *houghLine, &clustered_cells);

                // debug: draw preliminary clustering lines in blue
                // (original "hough line": red)
                #ifdef DEBUG
                    drawLines(clustered_cells, &clusterMat_V3, Scalar(255, 0, 0));
                #endif

                // Check for each component if it belongs to the current cluster.
                // Simultaneously, calculate the average MBR height
                // of all components in the cluster in order to refine
                // the rho resolution guess.
                for(auto clCellPos = clustered_cells.begin(); clCellPos != clustered_cells.end(); clCellPos++)
                {
                    for(auto component = comps->begin(); component != comps->end(); component++)
                    {
                        /*if(pointOnPolarLine((*component).centroid, *clCell, 1, &clusterMat_V3))
                        {
                            cluster.push_back(*component);
                            avgheight += (*component).mbr_max[0] - (*component).mbr_min[0];
                        }*/

                        // Look up component's accumulator positions
                        // and check if any of them match the clustered cells.
                        for(auto poslist = contributions.find((*component).centroid); poslist != contributions.end(); poslist++)
                        {
                            for(auto pos = (*poslist).second.begin(); pos != (*poslist).second.end(); pos++)
                            {
                                if((*pos) == (*clCellPos)[2])
                                {
                                    // Add component to cluster if it wasn't in there already
                                    if(find(cluster.begin(), cluster.end(), *component) == cluster.end())
                                        cluster.push_back(*component);
                                    else
                                        break;

                                    // calculate average height of the cluster iteratively
                                    avgheight += (*component).mbr_max[0] - (*component).mbr_min[0];
                                }
                            }
                        }
                    }
                }


                // 7) Compute new clustering factor
                // (= amount of rho cells to cluster)
                cluster.size() == 0 ? avgheight = avgheight : avgheight /= cluster.size(); // avoid division by zero
                int factor = avgheight / rho;

                // 8) Reset cluster and re-cluster using the new factor
                cluster.clear();
                clusterCells(factor, rho, numRho, numAngle, *houghLine, &clustered_cells);

                // Find all components whose accumulator cells belong to the cluster.
                for(auto clCellPos = clustered_cells.begin(); clCellPos != clustered_cells.end(); clCellPos++)
                {
                    for(auto component = comps->begin(); component != comps->end(); component++)
                    {
                        /*if(pointOnPolarLine((*component).centroid, *clLine, 1, &clusterMat_V3))
                        {
                            // associate current Hough line with the component
                            (*component).houghLine = *houghLine;

                            // add this component to the refined cluster
                            if(find(cluster.begin(), cluster.end(), *component) == cluster.end())
                                cluster.push_back(*component);
                        }*/

                        for(auto poslist = contributions.find((*component).centroid); poslist != contributions.end(); poslist++)
                        {
                            for(auto pos = (*poslist).second.begin(); pos != (*poslist).second.end(); pos++)
                            {
                                if((*pos) == (*clCellPos)[2])
                                {
                                    // associate current Hough line with the component
                                    (*component).houghLine = *houghLine;

                                    // add this component to the refined cluster
                                    // (and don't add it more than once)
                                    if(find(cluster.begin(), cluster.end(), *component) == cluster.end())
                                        cluster.push_back(*component);

                                    break;
                                }
                            }
                        }
                    }
                }


                // debug: show cluster MBRs---------------------------------------------
                for(auto comp = cluster.begin(); comp != cluster.end(); comp++)
                {
                    ConnectedComponent c = *comp;

                    // rectangle works with (col,row), so swap coordinates
                    Point min = Vec2i(c.mbr_min[1], c.mbr_min[0]);
                    Point max = Vec2i(c.mbr_max[1], c.mbr_max[0]);

                    // draw MBR for this component
                    rectangle(clusterMat_V3, min, max, Scalar(255, 0, 0), 1, 8, 0);
                }


                // debug: draw reclustered clustering lines in green and
                // original "hough line" in red
                #ifdef DEBUG
                    vector<Vec3f> oneline;
                    oneline.push_back(*houghLine);
                    drawLines(clustered_cells, &clusterMat_V3, Scalar(0, 255, 255));
                    drawLines(oneline, &clusterMat_V3, Scalar(0, 0, 255));

                    waitKey(0);
                #endif


                //------------------------------------------------------------------

                // 9) Mark those components for deletion which still belong to
                // the cluster even after the cluster area filter.
                // These components will be deleted when all hough lines for
                // this threshold have been evaluated to avoid destructive line overlap.
                if(cluster.size() != 0)
                {
                    // apply area ratio filter to eliminate large components from the cluster
                    // whose centroids are coincidentally on a string's hough line
                    clusterCompAreaFilter(&cluster, 5);
                }

                // debug: show cluster MBRs FILTERED ---------------------------------------------
                for(auto comp = cluster.begin(); comp != cluster.end(); comp++)
                {
                    ConnectedComponent c = *comp;
                    Point min = Vec2i(c.mbr_min[1], c.mbr_min[0]);
                    Point max = Vec2i(c.mbr_max[1], c.mbr_max[0]);
                    rectangle(clusterMat_V3, min, max, Scalar(0, 255, 0), 1, 8, 0);
                }

                namedWindow("CURRENT CLUSTER", WINDOW_AUTOSIZE);
                imshow("CURRENT CLUSTER", clusterMat_V3);

                #ifdef DEBUG
                    // reset cluster mat
                    cvtColor(input, clusterMat_V3, CV_GRAY2RGB);
                #endif
                //------------------------------------------------------------------

                // sort components in this cluster by their distance to the original hough line
                sort(cluster.begin(), cluster.end(), compareByLineDistance);

                // Calculate component meta information and store it
                if(cluster.size() != 0)
                {
                    CollinearString cs = CollinearString(cluster, avgheight);
                    cs.refine();
                    collinearStrings.push_back(cs);
                }

                // clustering for this hough line is done
                cluster.clear();

                // next cluster
                clusterNo++;
            }

            // All clusters for the current threshold have been evaluated.
            // Now, graphics can be deleted by using the meta information retrieved earlier.
            // Extra care is taken in order to not delete shorter strings before longer strings,
            // as this can lead to characters not being deleted ("destructive line overlap").
            for(auto costr = collinearStrings.begin(); costr != collinearStrings.end(); costr++)
                for(auto cophr = (*costr).phrases.begin(); cophr != (*costr).phrases.end(); cophr++)
                    for(auto cowrd = (*cophr).words.begin(); cowrd != (*cophr).words.end(); cowrd++)
                    {
                        CollinearGroup current = *cowrd;

                        // Words have to exceed a set component size and
                        // match or exceed the current threshold - since the threshold
                        // signalizes how long the string is an drops from
                        // highest (longest) to lowest (shortest).
                        if(current.size() >= threshold && current.size() > 1)
                        {
                            //cout << current.size() << " >= " << threshold << "\n";

                            for(auto coch = current.chars.begin(); coch != current.chars.end(); coch++)
                            {
                                // erase the pixels associated with the current
                                // component from the output image
                                eraseComponentPixels(*coch, &erased);

                                // debug
                                //printAccumValues (accumulator, numAngle+2, numRho+2);

                                // 10.) Delete those values from the accumulator which were contributed
                                // to it by components which are still in the cluster by now and thus
                                // are marked for deletion anyway
                                deleteLineContributions(accumulator, (*coch).centroid, contributions);

                                // debug
                                //cout << "\n\nAFTER:\n";
                                //printAccumValues(accumulator, numAngle+2, numRho+2);

                                // debug: reset cluster mat------------------
                                //cvtColor(erased, clusterMat_V3, CV_GRAY2RGB);
                                // ------------------------------------------
                            }
                        }
                    }


            // decrement accumulator threshold
            threshold--;

            // if this is the first pass (= nearly horizontal and vertical line detection)
            if(counter == 0)
            {
                // clear line buffer
                lines.clear();

                // extract all lines that now exactly meet the threshold
                // (they were lower than the previous threshold, and can now be admitted)
                HoughLinesExtract (accumulator, numRho, numAngle, rho, theta, 0., threshold, &lines, THRESH_EQ);
                HoughLinesExtract (accumulator, numRho, numAngle, rho, theta, 1.48353, threshold, &lines, THRESH_EQ);
                HoughLinesExtract (accumulator, numRho, numAngle, rho, theta, 3.05433, threshold, &lines, THRESH_EQ);

                cout << "LINESNOW: " << lines.size() << " with THRESHOLD: " << threshold << "\n";
            }

            // if this is the second pass (= 0 - 180° lines, i.e. all other angles)
            else
            {
                // reset line buffer and extract new lines
                lines.clear();
                HoughLinesExtract (accumulator, numRho, numAngle, rho, theta, 0., threshold, &lines, THRESH_EQ);

                cout << "LINESNOW: " << lines.size() << " with THRESHOLD: " << threshold << "\n";
            }
        }

        // first pass is done
        if(counter == 0)
        {
            cout << "\n --------- SECOND PASS. ---------- \n";

            // show post-first pass (vertical/horizontal) hough lines in blue
            //drawLines(lines, &showHough, Scalar(255, 0, 0));
            //drawLines(lines, &hough_8U, Scalar(255, 0, 0));

            // reset threshold
            threshold = 20;

            // reset line buffer
            lines.clear();

            // calculate hough domain for lines with angles [0°, 180°]
            HoughLinesCustom(hough_UC, rho, theta, 0., 3.14159, accumulator, &contributions);
            HoughLinesExtract (accumulator, numRho, numAngle, rho, theta, 0., threshold, &lines, THRESH_GT);

            cout << "LINESNOW: " << lines.size() << " with THRESHOLD: " << threshold << "\n";


            // do one last iteration
            counter++;
        }

        // second pass done, end algorithm
        else
        {
            // show all 0 - 180° hough lines in red
            //drawLines(lines, &showHough, Scalar(0, 0, 255));
            //drawLines(lines, &hough_8U, Scalar(255, 0, 0));

            counter++;
        }
    }

    // show hough lines overlayed on image
    //namedWindow("HOUGH+IMAGE", WINDOW_AUTOSIZE);
    //imshow("HOUGH+IMAGE", showHough);

    // show hough lines on component centroids
    namedWindow("CENTROIDS", WINDOW_AUTOSIZE);
    imshow("CENTROIDS", hough_UC);

    // show result
    namedWindow("WITHOUT TEXT", WINDOW_AUTOSIZE);
    imshow("WITHOUT TEXT", erased);

    // show clustering
    //imshow("CLUSTER", clusters);

    // cleanup
    delete [] accumulator;

    waitKey(0);
}
