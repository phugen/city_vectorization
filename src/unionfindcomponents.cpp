#include "include/unionfindcomponents.hpp"
#include "include/unionfind.hpp"
#include "include/auxiliary.hpp"
#include "include/colorconversions.hpp"

#include <iostream>
#include <set>

using namespace std;
using namespace cv;


// Two-pass connected-component finding.
// (https://en.wikipedia.org/wiki/Connected-component_labeling)
//
// Expects binary picture (e.g. black layer)
void unionFindComponents(Mat input, vector<ConnectedComponent>* components)
{
    const int rows = input.rows; // shortcuts
    const int cols = input.cols;

    int label = 1; // number of first component
    int* labels; // keeps track of which pixel belongs to which component
    int* pxPerLabel; // number of black pixels in label
    Vec2i* seedPerLabel; // seed[label] = coordinates of the first black pixel in the component

    UnionFind* uf; // union-find data structure
    vector<Vec2i> neighborPositions; // (x,y) positions of all black neighbors of current pixel
    vector<int>* nb_labels; // contains the labels of the neighbors of a pixel
    set<int>* trueLabels; // contains the actually valid labels.

    Vec2i** MBRCoords; // contains min- and max points of each component's MBR



    // initialize labels
    // and assign all pixels the "background" label, "0"
    labels = new int [rows * cols];

    for(int i = 0; i < rows; i++)
        for(int j = 0; j < cols; j++)
            labels[i * cols + j] = 0;


    // initialize neighbor labels
    nb_labels = new vector<int>();

    // initialize trueLabels
    trueLabels = new set<int>();

    // initialize black pixel count per label
    pxPerLabel = new int[rows * cols];
    for(int i = 0; i < (rows * cols); i++)
        pxPerLabel[i] = 0;


    // initialize label->seed lookup table
    // with dummy values
    seedPerLabel = new Vec2i[sizeof(labels) / sizeof(int)];
    for (size_t i = 0; i < (int) (sizeof(labels) / sizeof(int)); i++)
    {
        seedPerLabel[i] = Vec2i(-1, -1);
    }


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

                    // update label, seed and black pixel count
                    // for this label
                    labels[i * cols + j] = label;
                    seedPerLabel[label] = Vec2i(j, i);
                    pxPerLabel[label] = 1;

                    // next black pixel could be a new label
                    label += 1;
                }

                else
                {
                    // reset list of neighbor labels
                    nb_labels->clear();

                    // begin with the first label ...
                    Vec2i first = neighborPositions.front();
                    int minLabel = labels[first[0] * cols + first[1]];

                    // look at all neighbors' labels
                    for(vector<Vec2i>::iterator nbpIter = neighborPositions.begin(); nbpIter != neighborPositions.end(); nbpIter++)
                    {
                        Vec2i nPos = *nbpIter;

                        // save labels for each neighbor
                        nb_labels->push_back(labels[nPos[0] * cols + nPos[1]]);

                        // find smallest label
                        minLabel = min(minLabel, labels[nPos[0] * cols + nPos[1]]);
                    }

                    // assign smallest label to current pixel, update seed
                    // and increase black pixel counter
                    labels[i * cols + j] = minLabel;
                    seedPerLabel[minLabel] = Vec2i(j, i);
                    pxPerLabel[minLabel] += 1;

                    // if the newly labelled pixel changes the MBR
                    // of that label's region, update it
                    MBRCoords[minLabel][0][0] = min(MBRCoords[minLabel][0][0], i);
                    MBRCoords[minLabel][0][1] = min(MBRCoords[minLabel][0][1], j);
                    MBRCoords[minLabel][1][0] = max(MBRCoords[minLabel][1][0], i);
                    MBRCoords[minLabel][1][1] = max(MBRCoords[minLabel][1][1], j);

                    // merge components
                    for (vector<int>::iterator nblIter = nb_labels->begin(); nblIter != nb_labels->end(); nblIter++)
                        uf->merge(minLabel, *nblIter);
                }
            }


    // Second pass: Translate labels (merge equivalent labels) by searching for
    // the set that contains the label and then using it as the "true" label.
    for (int i = 0; i < rows; i++)
        for(int j = 0; j < cols; j++)
        {
            int oldLabel = labels[i * cols + j];
            int newLabel = uf->find(labels[i * cols + j]);

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

                // update seed; since it doesn't matter which
                // one is chosen, just choose the new one
                //
                // actually, this should be useless. TODO
                seedPerLabel[oldLabel] = seedPerLabel[newLabel];

                // and add one to the new label's pixel count
                // for every pixel that was changed
                pxPerLabel[newLabel] += 1;
            }

            // update label value and record actually used labels
            labels[i * cols + j] = uf->find(labels[i * cols + j]);
            trueLabels->insert(uf->find(labels[i * cols + j]));
        }

    cout << "SECOND PASS DONE" << "\n\n";


    int numTrueComponents = trueLabels->size();
    cout << "Number of components found: " << numTrueComponents << "\n";


    Mat showMBR;
    cvtColor(input, showMBR, CV_GRAY2RGB);

    // color found components
    for (int i = 0; i < rows; i++)
        for(int j = 0; j < cols; j++)
        {
            if(labels[i * cols + j] != 0)
            {
                Vec3b color = intToRGB(Vec2i(0, numTrueComponents), labels[i * cols + j]);
                showMBR.at<Vec3b>(i, j) = color;
            }

            else
                showMBR.at<Vec3b>(i, j) = showMBR.at<Vec3b>(i, j);
        }



    // retrieve MBRs and show them
    for(set<int>::iterator iter = trueLabels->begin(); iter != trueLabels->end(); iter++)
    {
        // skip coordinates of invalid sets
        if(!isValidCoord(MBRCoords[*iter]))
            continue;

        // create connected component
        // and store it in vector
        components->push_back(*(new ConnectedComponent(MBRCoords[*iter][0], MBRCoords[*iter][1], pxPerLabel[*iter], seedPerLabel[*iter])));

        // rectangle works with (col,row), so swap coordinates
        Point min = Vec2i(MBRCoords[*iter][0][1], MBRCoords[*iter][0][0]);
        Point max = Vec2i(MBRCoords[*iter][1][1], MBRCoords[*iter][1][0]);

        // draw MBR for this component
        rectangle(showMBR, min, max, Scalar(0, 0, 255), 1, 8, 0);
    }

    // cleanup
    delete [] labels;
    /*delete [] seedPerLabel;
    delete [] pxPerLabel;
    delete [] MBRCoords;
    delete uf;
    delete nb_labels;
    delete trueLabels;*/

    // show result
    namedWindow("Components", WINDOW_AUTOSIZE);
    imshow("Components", showMBR);
}
