/**
  * Implements a raster-to-vector graphic transformation
  * and outputs the vector graphic as an .svg.
  *
  * Author: phugen
  */

#include "include/vectorization/vectorize.hpp"
#include "include/cairo/cairo.h"
#include "include/cairo/cairo-svg.h"
#include "include/vectorization/zhangsuen.hpp"
#include "include/vectorization/moore.hpp"

#include <iostream>

using namespace std;
using namespace cv;


// Create a .svg file "filename.svg" based on the extracted vector lines.
void vectorsToFile (Mat* image, set<vectorLine*> refinedLines, string filename)
{
    namedWindow("before", WINDOW_NORMAL);
    imshow("before", *image);

    // debug: overlay found lines on image in blue
    Mat vectoronly = Mat(image->rows, image->cols, image->type());
    cvtColor(*image, vectoronly, CV_GRAY2RGB);

    for(auto l = refinedLines.begin(); l != refinedLines.end(); l++)
    {
        vectorLine* li = *l;
        pixel* start = li->getStart();
        pixel* end = li->getEnd();

        Point pt1 = Point(start->coord[1], start->coord[0]);
        Point pt2 = Point(end->coord[1], end->coord[0]);

        line(vectoronly, pt1, pt2, Scalar(255, 0, 0), 1);
    }

    namedWindow("VECTORS", WINDOW_NORMAL);
    imshow("VECTORS", vectoronly);

    // Set up cairo canvas
    cairo_surface_t *surface;
    cairo_t *cr;
    surface = cairo_svg_surface_create((filename + ".svg").c_str(), image->cols, image->rows);
    cr = cairo_create(surface);
    cairo_set_source_rgb(cr, 0, 0, 0); // set background color
    cairo_set_line_width(cr, 1); // set line width
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE); // don't smoothen line ends

    // Write line segment descriptions to .svg file
    for(auto l = refinedLines.begin(); l != refinedLines.end(); l++)
    {
        vectorLine* li = *l;
        pixel* start = li->getStart();
        pixel* end = li->getEnd();

        // cairo can't handle 1px paths with square line caps
        // so draw a 1px square instead!
        if(start->coord == end->coord)
        {
            // use "half" pixels to get squares that don't stretch
            // over the border between two pixels if coordinate is odd
            //if(start->coord[1] % 2 == 1)
                cairo_rectangle(cr, start->coord[1]-0.5, start->coord[0]-0.5, 1, 1);

            //else
            //    cairo_rectangle(cr, start->coord[1], start->coord[0], 1, 1);

            cairo_fill (cr);
        }

        // draw normal line
        else
        {
            cairo_move_to(cr, start->coord[1], start->coord[0]);
            cairo_line_to(cr, end->coord[1], end->coord[0]);
            cairo_stroke(cr);
        }
    }

    // Free canvas
    cairo_surface_destroy(surface);
    cairo_destroy(cr);
}


// Returns all lines that the input pixel
// is connected to
vector<vectorLine*> connectedTo (int height, int width, pixel* cur, vector<pixel*>* pixels)
{
    vector<pixel*> toCheck;

    // boundary checks as usual (should be cleaned up - indices only
    // so it can be used in all places where this is needed)
    int i = cur->coord[0]; // y-coordinate
    int j = cur->coord[1]; // x-coordinate


    // top left pixel
    if(i == 0 && j == 0)
    {
        toCheck.push_back(pixels->at(i * width + j+1));
        toCheck.push_back(pixels->at(i+1 * width + j+1));
        toCheck.push_back(pixels->at(i+1 * width + j));
    }

    // top right pixel
    else if(i == 0 && j == width)
    {
        toCheck.push_back(pixels->at(i+1 * width + j));
        toCheck.push_back(pixels->at(i+1 * width + j-1));
        toCheck.push_back(pixels->at(i * width + j-1));
    }


    // bottom left pixel
    else if (i == height && j == 0)
    {
        toCheck.push_back(pixels->at(i-1 * width + j+1));
        toCheck.push_back(pixels->at(i * width + j+1));
        toCheck.push_back(pixels->at(i-1 * width + j));
    }

    // bottom right pixel
    else if (i == height && j == width)
    {
        toCheck.push_back(pixels->at(i * width + j-1));
        toCheck.push_back(pixels->at(i-1 * width + j-1));
        toCheck.push_back(pixels->at(i-1 * width + j));
    }

    // upper border
    else if (i == 0)
    {
        toCheck.push_back(pixels->at(i * width + j+1));
        toCheck.push_back(pixels->at(i+1 * width + j+1));
        toCheck.push_back(pixels->at(i+1 * width + j));
        toCheck.push_back(pixels->at(i+1 * width + j-1));
        toCheck.push_back(pixels->at(i * width + j-1));
    }

    // left border
    else if (j == 0)
    {
        toCheck.push_back(pixels->at(i-1 * width + j+1));
        toCheck.push_back(pixels->at(i * width + j+1));
        toCheck.push_back(pixels->at(i+1 * width + j+1));
        toCheck.push_back(pixels->at(i+1 * width + j));
        toCheck.push_back(pixels->at(i-1 * width + j));
    }


    // right border
    else if (j == width)
    {
        toCheck.push_back(pixels->at(i+1 * width + j));
        toCheck.push_back(pixels->at(i+1 * width + j-1));
        toCheck.push_back(pixels->at(i * width + j-1));
        toCheck.push_back(pixels->at(i-1 * width + j-1));
        toCheck.push_back(pixels->at(i-1 * width + j));
    }

    // lower border
    else if (i == height)
    {
        toCheck.push_back(pixels->at(i-1 * width + j+1));
        toCheck.push_back(pixels->at(i * width + j+1));
        toCheck.push_back(pixels->at(i * width + j-1));
        toCheck.push_back(pixels->at(i-1 * width + j-1));
        toCheck.push_back(pixels->at(i-1 * width + j));
    }

    // Normal case - pixel is somewhere
    // in the image, but not near any border.
    else
    {
        toCheck.push_back(pixels->at(i-1 * width + j+1));
        toCheck.push_back(pixels->at(i * width + j+1));
        toCheck.push_back(pixels->at(i+1 * width + j+1));
        toCheck.push_back(pixels->at(i+1 * width + j));
        toCheck.push_back(pixels->at(i+1 * width + j-1));
        toCheck.push_back(pixels->at(i * width + j-1));
        toCheck.push_back(pixels->at(i-1 * width + j-1));
        toCheck.push_back(pixels->at(i-1 * width + j));
    }

    vector<vectorLine*> connected;

    // find all lines which have nodes that neighbor
    // the current node
    for(auto pixel = toCheck.begin(); pixel != toCheck.end(); pixel++)
    {
        // if the pixel is not a white pixel
        if((*pixel)->line != NULL)
            connected.push_back((*pixel)->line);
    }

    return connected;
}

// Checks if a pixel is a true node
// (exactly one line or more than two lines connected)
bool isTrue(vector<vectorLine*> connected)
{
    if((connected.size() == 1) || (connected.size() > 2))
        return true;

    else
        return false;
}


// Applies the Ramer-Douglas-Peucker algorithm
// for approximating vector lines, so fewer nodes are used.
vector<vector<vectorLine*>> refineVectors (Mat* image, map<pixel*, vectorLine*>* nodeToLine, vector<pixel*>* pixels)
{
    int height = image->rows - 1;
    int width = image->cols - 1;

    pixel* dummy = new pixel(Vec2i(-1, -1), NULL, false);
    set<vectorLine*> lines;
    for(auto px = nodeToLine->begin(); px != nodeToLine->end(); px++)
    {
        lines.insert((*px).second);
    }

    // Walk through all lines via DFS and
    // find sub-paths that can be refined.
    //
    // Since graphs are usually not made up of
    // only one component, this implies that the
    // DFS needs to be applied as long as there are
    // lines that are still unwalked.
    //
    // A path's start and end nodes are "true" nodes, as defined in
    // the Moore paper. True nodes are those nodes which
    // connect to exactly one line or more than two lines.
    //
    // Once a true end node is reached, the path so far is recorded as
    // a set of lines, and the algorithm deletes all found lines from
    // the search, leaving the other sub-paths which branch off
    // at true nodes with (previously) more than three connecting lines.

    vector<vector<vectorLine*>> refinedPaths; // output list

    // loop until all graphs in the image
    // have been traversed
    while(lines.size() != 0)
    {
        set<vectorLine*> found; // set of found lines that still have to be walked
        set<vectorLine*> closed; // set of found lines that have been walked already

        found.insert(*lines.begin());

        // keep going until all lines in this subgraph were walked
        while(found.size() != 0) // FOUND VS LINES? ERASING WHERE? && closed.find(currentLine) != closed.end()
        {
            // start at line with smallest start point
            // (should be the < order on vectorLines)
            vectorLine* current = *found.begin();
            found.erase(current);
            closed.insert(current);

            // find all lines that are connected to this line's end point
            // MUST NOT FIND CLOSED LINES. <--- deal with circular paths
            vector<vectorLine*> connected = connectedTo((image->rows) - 1, (image->cols) - 1, current->getEnd(), pixels);

            // if the last endpoint was a true node
            if(isTrue(connected))
            {

            }

            // add these lines to the found set
            for(auto l = connected.begin(); l != connected.end(); l++)
                found.insert(*l);
        }
    }

    delete dummy;
    // delete pixels;
    // delete original lines?

    return refinedPaths;
}


// Initialize all pixels with their coordinates
// in the picture.
void initPixels(vector<pixel*>* pixels, Mat* image)
{
    for(int i = 0; i < image->rows; i++)
    {
        for(int j = 0; j < image->cols; j++)
        {
            pixels->at(i * image->cols + j) = new pixel(Vec2i(i, j), NULL, false);
        }
    }
}

// Expects binary image (blackLayer)
void vectorizeImage (Mat* image, string filename)
{
    cout << "\n ------------------------- \n";
    cout << "Starting vectorization ...\n";

    map<pixel*, vectorLine*> nodeToLine; // provides a endpoint->line mapping
    vector<vector<vectorLine*>> refinedPaths; // holds results of douglas-peucker algorithm

    vector<pixel*>* pixels = new vector<pixel*>((image->rows + 2) * (image->cols + 2)); // states of all pixels (+ dummy values for 1px border)
    pixel* dummy = new pixel(Vec2i(-1, -1), NULL, false);
    initPixels(pixels, image);

    Mat inverted = Mat(image->rows, image->cols, image->type());
    Mat thinned = Mat(image->rows, image->cols, image->type());

    // thin image using Zhang-Suen
    cout << "Extracting image skeleton... \n";
    bitwise_not(*image, inverted);
    thinning(inverted);
    bitwise_not(inverted, thinned);

    imwrite("thinned.png", thinned);

    // create vector lines
    cout << "Extracting vectors from raster image... \n";
    nodeToLine = mooreVector(thinned, pixels, dummy);

    // refine vectors by removing unnecessary nodes
    cout << "Refining vector data... \n";
    //refinedPaths = refineVectors(image, &nodeToLine, pixels);


    // debug: extract lines without refinement
    set<vectorLine*> fake;
    for(auto l = nodeToLine.begin(); l != nodeToLine.end(); l++)
        fake.insert((*l).second);

    // recover image topology and add color
    cout << "Recovering image topology... \n";
    // recoverTopology(lines);

    // write vector lines to file
    cout << "Writing vector data to file << " << filename << ".svg...\n";
    vectorsToFile(&thinned, fake, filename);

    // delete allocated line objects
    cout << "Cleaning up... \n";
    /*for(auto l = refinedLines.begin(); l != refinedLines.end(); l++)
        delete (*l);

    for(auto l = nodeToLine.begin(); l != nodeToLine.end(); l++)
        delete (*l).second;

    // deallocate pixel objects
    for(auto l = nodeToLine.begin(); l != nodeToLine.end(); l++)
        delete (*l).first;*/

    cout << "VECTORIZATION DONE!\n";
}



