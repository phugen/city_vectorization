/**
  * Implements a raster-to-vector graphic transformation
  * and outputs the vector graphic as an .svg.
  *
  * Uses an R-Tree implementation due to
  * http://superliminal.com/sources/sources.htm
  *
  * Author: phugen
  */

#include "include/vectorize.hpp"
#include "include/cairo/cairo.h"
#include "include/cairo/cairo-svg.h"
#include "include/RTree.hpp"

#include <map>
#include <iostream>

using namespace std;
using namespace cv;

// Create a .svg file "filename.svg" based on the input raster image.
// Expects binary image (i.e. image passed blacklayer filter before).
void vectorizeImage (Mat* image, string filename, int maxDist)
{
    cout << "\n ------------------------- \n";
    cout << "Starting vectorization ...\n";

    // Invert image; canny not needed
    Mat inverted;
    bitwise_not(*image, inverted);

    // Find line segments
    vector<Vec4f> lines;
    HoughLinesP(inverted, lines, 1, 0.0174533, 15, 0, 0);

    cout << lines.size() << " line segments detected!\n";

    // debug: overlay found lines on image in blue
    cvtColor(*image, *image, CV_GRAY2RGB);
    Mat vectoronly = Mat(image->rows, image->cols, image->type());

    for(auto l = lines.begin(); l != lines.end(); l++)
    {
        Vec4f cur = *l;
        Point pt1 = Point(cur[0], cur[1]);
        Point pt2 = Point(cur[2], cur[3]);

        line(vectoronly, pt1, pt2, Scalar(255, 0, 0), 1);
    }

    // debug: show result of hough transform
    namedWindow("VECTORIZED", WINDOW_NORMAL);
    imshow("VECTORIZED", vectoronly);

    cout << "Refining vector data (max. join limit is " << maxDist << " pixels) ... \n";

    // Attempt to join short segments and close gaps
    // Destructive edge linking
    refineVectors(&lines, maxDist, NULL);

    cout << "Writing segment descriptions to " << filename << ".svg ...\n";

    // Set up cairo canvas
    cairo_surface_t *surface;
    cairo_t *cr;
    surface = cairo_svg_surface_create((filename + ".svg").c_str(), image->cols, image->rows);
    cr = cairo_create(surface);
    cairo_set_source_rgb(cr, 0, 0, 0); // set background color
    cairo_set_line_width(cr, 1); // set line width
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE); // don't smoothen line ends

    // Write line segment descriptions to .svg file
    for(auto l = lines.begin(); l != lines.end(); l++)
    {
        Vec4f cur = *l;

        cairo_move_to(cr, cur[0], cur[1]);
        cairo_line_to(cr, cur[2], cur[3]);
        cairo_stroke(cr);
    }

    // Constructive edge linking
    //refineVectors(&lines, maxDist, cr);

    // Free canvas
    cairo_surface_destroy(surface);
    cairo_destroy(cr);

    cout << "Vectorization done!\n";
}

// The arguments for the R-Tree callback function.
struct rtree_arg
{
    map<int, Vec2f>* hashmap; // maps R-Tree IDs to point coordinates
    int maxDist; // for distance checks within the MBR
    Vec4f input; // line points of input point's line
    Vec2f found; // the nearest neighbor that is <= maxDist away
    int found_id; // id of the found point
};

// Callback function for the R-Tree.
// Stores the nearest neighbor in the argument struct.
bool rCallback(int id, void* arg)
{
    struct rtree_arg* args = (struct rtree_arg*) arg;

    // perform lookup in hashmap
    Vec2f maybe = (*(args->hashmap->find(id))).second;

    // check distance from input point to found point;
    // because a rectangle is used, the distance could
    // be > maxDist, even if the point is inside the rectangle
    double dist = sqrt(pow(args->input[0] - maybe[0], 2) + pow(args->input[1] - maybe[1], 2));

    // adhere to max dist threshold and don't find own line points
    if((dist <= args->maxDist) &&
       (args->input[0] != args->found[0] && args->input[1] != args->found[1]) &&
       (args->input[2] != args->found[0] && args->input[3] != args->found[1]))
    {
        //cout << dist <<  " \n";
        args->found = maybe;
        args->found_id = id;
    }

    else
        return true; // keep searching

    return false; // we only care about the nearest neighbor!
}

// Attempts to join lines in order to close gaps
// and decrease the total line count.
// maxDist is the maximum acceptable distance for linking
// two segments.
void refineVectors (vector<Vec4f>* lines, int maxDist, cairo_t* cr)
{
    // R-Tree for starting points: int IDs (can't handle Vec2f - maybe array?), float values, 2 dimensions.
    RTree<int, float, 2, float> start_tree;
    map<int, Vec2f> id_to_pos; // id -> (x,y) lookup table

    // set arguments for R-tree callback function
    struct rtree_arg arg;
    arg.maxDist = maxDist;
    arg.hashmap = &id_to_pos;

    // initialize tree with all starting points of all
    // found segments
    int count = 0;
    for(int i = 0; i < (int) lines->size(); i++, count+=2)
    {
        Vec4f cur = lines->at(i);
        const float MBR_min[2] = {cur[0], cur[1]};
        const float MBR_max[2] = {cur[0], cur[1]};
        const float MBR_min2[2] = {cur[2], cur[3]};
        const float MBR_max2[2] = {cur[2], cur[3]};

        start_tree.Insert(MBR_min, MBR_max, count);
        start_tree.Insert(MBR_min2, MBR_max2, count+1);
        arg.hashmap->insert(make_pair(count, Vec2f(cur[0], cur[1]))); // add id to lookup table
        arg.hashmap->insert(make_pair(count+1, Vec2f(cur[2], cur[3])));
    }

    // for each ending point: query R-Tree and find closest
    // starting point that that obeys the maxDist threshold
    for(auto l = lines->begin(); l != lines->end(); l++)
    {   
        // construct search MBR around the current endpoint
        const float MBR_min[2] = {(*l)[2] - maxDist, (*l)[3] - maxDist};
        const float MBR_max[2] = {(*l)[2] + maxDist, (*l)[3] + maxDist};

        // set current endpoints as search input
        arg.input = Vec4f((*l)[0], (*l)[1], (*l)[2], (*l)[3]);

        // initialize found vector to dummy value
        // in case no points are within maxDist
        arg.found = Vec2f(-1, -1);

        // Search for the closest start point
        start_tree.Search(MBR_min, MBR_max, rCallback, (void*) &arg);

        // if a point was found, perform edge linking
        if(arg.found[0] != -1)
        {
            // update R-Tree by removing
            // the old point - the updated point
            // is already in the tree!
            const float MBR_min2[2] = {(*l)[2], (*l)[3]};
            const float MBR_max2[2] = {(*l)[2], (*l)[3]};


            Vec4f c = *l;
            //cout << c[0] << "," << c[1] << "->" << c[2] << "," << c[3] << " wird zu " << c[0] << "," << c[1] << "->" << arg.found[0] << "," << arg.found[1] << "\n";
            //waitKey(0);

            // Pre-emptive "destructive" edge linking: Change endpoint to newly found point
            (*l)[2] = arg.found[0];
            (*l)[3] = arg.found[1];

            start_tree.Remove(MBR_min2, MBR_max2, arg.found_id);

            // Postprocessive constructive edge linking:
            // Insert new line between endpoints
            /*cairo_move_to(cr, arg.input[2], arg.input[3]);
            cairo_line_to(cr, arg.found[0], arg.found[1]);
            cairo_stroke(cr);*/
        }
    }


    // check if the next line's starting point
    // is close the the current endpoint; If yes,
    // replace the endpoint of the current line by
    // the next line's starting point.
    /*if((l+1) != lines.end())
    {
        Vec4f next = *(l+1);

        // use euclidean distance to compare cur.end and next.start
        double dist = sqrt(pow((cur[2] - next[0]), 2) + pow((cur[3] - next[1]), 2));

        // join lines
        if(dist <= maxDist)
        {
            cur[2] = next[0];
            cur[3] = next[1];
        }
    }*/
}
