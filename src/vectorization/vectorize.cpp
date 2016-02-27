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

// Applies the Ramer-Douglas-Peucker algorithm
// for approximating lines with fewer nodes
void refineVectors (map<pixel*, vectorLine*>* nodeToLine)
{
    pixel* dummy = new pixel(Vec2i(-1, -1), NULL, false);

    // Walk through all lines via DFS and
    // find sub-paths that can be refined.
    // Since graphs are usually not made up of
    // only one component, this implies that the
    // DFS needs to be applied as long as there are
    // lines that are still unwalked.

    set<vectorLine*> open; // set of found lines that still have to be walked
    set<vectorLine*> closed; // set of found lines that have been walked already

    //while(lines->size() != 0)
    {

    }

    delete dummy;
    // delete pixels;
    // delete original lines?
}


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

        cairo_move_to(cr, start->coord[1], start->coord[0]);
        cairo_line_to(cr, end->coord[1], end->coord[0]);
        cairo_stroke(cr);
    }

    // Free canvas
    cairo_surface_destroy(surface);
    cairo_destroy(cr);
}


// Expects binary image (blackLayer)
void vectorizeImage (Mat* image, string filename)
{
    cout << "\n ------------------------- \n";
    cout << "Starting vectorization ...\n";

    map<pixel*, vectorLine*> nodeToLine; // provides a endpoint->line mapping
    set<vectorLine*> refinedLines; // holds results of douglas-peucker algorithm

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
    nodeToLine = mooreVector(thinned);

    // refine vectors by removing unnecessary nodes
    cout << "Refining vector data... \n";
    //refinedLines = refineVectors(&nodeToLine);

    // debug: extract lines without refinement
    for(auto l = nodeToLine.begin(); l != nodeToLine.end(); l++)
        refinedLines.insert((*l).second);

    // recover image topology and add color
    cout << "Recovering image topology... \n";
    // recoverTopology(lines);

    // write vector lines to file
    cout << "Writing vector data to file << " << filename << ".svg...\n";
    vectorsToFile(&thinned, refinedLines, filename);

    // delete allocated line objects
    cout << "Cleaning up... \n";
    for(auto l = refinedLines.begin(); l != refinedLines.end(); l++)
        delete (*l);

    for(auto l = nodeToLine.begin(); l != nodeToLine.end(); l++)
        delete (*l).second;

    // deallocate pixel objects
    for(auto l = nodeToLine.begin(); l != nodeToLine.end(); l++)
        delete (*l).first;

    cout << "VECTORIZATION DONE!\n";
}



