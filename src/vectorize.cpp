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
#include "include/zhangsuen.hpp"

#include <map>
#include <set>
#include <cstdint>
#include <iostream>

using namespace std;
using namespace cv;

// Functor for Vec2i comparison.
struct vec2i_compare
{
    bool operator () (Vec2i a, Vec2i b) const
    {
        return a[0] < b[0] || (a[0] == b[0] && a[1] < b[1]);
    }
};


// Returns a 8-bit encoding of the neighborhood
// of the current pixel in the image that can be
// interpreted like this: (c is the current pixel)
//
//    64  128  1
//    32   c   2
//    16   8   4
//
// The bit corresponding to each position is = 1 if
// the pixel at that position is 1, = 0 otherwise.
// "Out-of-image" pixels are treated as 0. (check if this is ok!)
//
// Expects a binary matrix in which line pixels are black (= 0).
uint8_t encodeNeighbors (Mat* image, Vec2i curPixel)
{
    int rows = image->rows;
    int cols = image->cols;
    int i = curPixel[0]; // y-coordinate
    int j = curPixel[1]; // x-coordinate
    uint8_t encoding = 0;


    // top left pixel
    if(i == 0 && j == 0)
    {
        if(image->at<uchar>(i, j+1) == 0) encoding += 2;
        if(image->at<uchar>(i+1, j+1) == 0) encoding += 4;
        if(image->at<uchar>(i+1, j) == 0) encoding += 8;
    }

    // top right pixel
    else if(i == 0 && j == (cols - 1))
    {
        if(image->at<uchar>(i+1, j) == 0) encoding += 8;
        if(image->at<uchar>(i+1, j-1) == 0) encoding += 16;
        if(image->at<uchar>(i, j-1) == 0) encoding += 32;
    }


    // bottom left pixel
    else if (i == (rows - 1) && j == 0)
    {
        if(image->at<uchar>(i-1, j+1) == 0) encoding += 1;
        if(image->at<uchar>(i, j+1) == 0) encoding += 2;
        if(image->at<uchar>(i-1, j) == 0) encoding += 128;
    }

    // bottom right pixel
    else if (i == (rows - 1) && j == (cols - 1))
    {
        if(image->at<uchar>(i, j-1) == 0) encoding += 32;
        if(image->at<uchar>(i-1, j-1) == 0) encoding += 64;
        if(image->at<uchar>(i-1, j) == 0) encoding += 128;
    }

    // upper border
    else if (i == 0)
    {
        if(image->at<uchar>(i, j+1) == 0) encoding += 2;
        if(image->at<uchar>(i+1, j+1) == 0) encoding += 4;
        if(image->at<uchar>(i+1, j) == 0) encoding += 8;
        if(image->at<uchar>(i+1, j-1) == 0) encoding += 16;
        if(image->at<uchar>(i, j-1) == 0) encoding += 32;
    }

    // left border
    else if (j == 0)
    {
        if(image->at<uchar>(i-1, j+1) == 0) encoding += 1;
        if(image->at<uchar>(i, j+1) == 0) encoding += 2;
        if(image->at<uchar>(i+1, j+1) == 0) encoding += 4;
        if(image->at<uchar>(i+1, j) == 0) encoding += 8;
        if(image->at<uchar>(i-1, j) == 0) encoding += 128;
    }


    // right border
    else if (j == (cols - 1))
    {
        if(image->at<uchar>(i+1, j) == 0) encoding += 8;
        if(image->at<uchar>(i+1, j-1) == 0) encoding += 16;
        if(image->at<uchar>(i, j-1) == 0) encoding += 32;
        if(image->at<uchar>(i-1, j-1) == 0) encoding += 64;
        if(image->at<uchar>(i-1, j) == 0) encoding += 128;
    }

    // lower border
    else if (i == (rows - 1))
    {
        if(image->at<uchar>(i-1, j+1) == 0) encoding += 1;
        if(image->at<uchar>(i, j+1) == 0) encoding += 2;
        if(image->at<uchar>(i, j-1) == 0) encoding += 32;
        if(image->at<uchar>(i-1, j-1) == 0) encoding += 64;
        if(image->at<uchar>(i-1, j) == 0) encoding += 128;
    }

    // Normal case - pixel is somewhere
    // in the image, but not near any border.
    else
    {
        if(image->at<uchar>(i-1, j+1) == 0) encoding += 1;
        if(image->at<uchar>(i, j+1) == 0) encoding += 2;
        if(image->at<uchar>(i+1, j+1) == 0) encoding += 4;
        if(image->at<uchar>(i+1, j) == 0) encoding += 8;
        if(image->at<uchar>(i+1, j-1) == 0) encoding += 16;
        if(image->at<uchar>(i, j-1) == 0) encoding += 32;
        if(image->at<uchar>(i-1, j-1) == 0) encoding += 64;
        if(image->at<uchar>(i-1, j) == 0) encoding += 128;
    }

    return encoding;
}

// add rule "rule" to each table entry in the neighborhoods
// list, and then clears the list.
void addToTable (vector<int>* neighborhoods, int* ruleTable, int rule)
{
    for(auto nbh = neighborhoods->begin(); nbh != neighborhoods->end(); nbh++)
        ruleTable[*nbh] = rule;

    neighborhoods->clear();
}

// Sets the rule to apply for each neighborhood.
// Unset rules are rule 1, i.e. do nothing.
void initRuleTable(int* ruleTable)
{
    // initialize all rules to rule 1
    //memset((void*) ruleTable, 1, sizeof(int) * 256);
    ruleTable[0] = 1;

    // rules set as given in the paper:

    // rule 2:
    vector<int> temp = {2, 3, 4, 6, 7, 8, 10, 11, 12, 14, 15, 16, 18,
                       19, 20, 22, 23, 24, 26, 27, 28, 30, 31};
    addToTable(&temp, ruleTable, 2);

    // rule 3:
    ruleTable[1] = 3;
    ruleTable[21] = 3;

    // rule 4:
    temp = {32, 42, 43, 46, 47, 48, 56, 58, 59, 60, 62, 63, 96, 106,
           107, 110, 111, 112, 120, 122, 123, 124, 126, 127};
    addToTable(&temp, ruleTable, 4);

    // rule 5:
    temp = {64, 74, 75, 78, 79, 82, 83, 84, 86, 87, 88, 90, 91, 92,
            94, 95};
    addToTable(&temp, ruleTable, 5);

    // rule 6:
    temp = {128, 129, 131, 135, 138, 139, 142, 143, 146, 147, 148,
           149, 150, 151, 154, 155, 158, 159, 192, 193, 195, 199,
           202, 203, 206, 207, 210, 211, 212, 213, 214, 215, 218,
           219, 222, 223, 200};
    addToTable(&temp, ruleTable, 6);

    // rule 7:
    temp = {5, 9, 13, 17, 25, 29};
    addToTable(&temp, ruleTable, 7);

    // rule 8:
    temp = {34, 35, 36, 38, 39, 40, 44, 50, 51, 52, 54, 55, 98, 99,
           100, 102, 103, 104, 108, 114, 115, 116, 118, 119};
    addToTable(&temp, ruleTable, 8);

    // rule 9:
    temp = {66, 67, 68, 70, 71, 72, 76, 80};
    addToTable(&temp, ruleTable, 9);

    // rule 10:
    temp = {130, 132, 133, 134, 136, 137, 140, 141, 144, 145, 152,
           153, 156, 157, 194, 196, 197, 198, 201, 204, 205,
           208, 209, 216, 217, 220, 221};
    addToTable(&temp, ruleTable, 10);

    // rule 11:
    temp = {33, 49, 97, 113};
    addToTable(&temp, ruleTable, 11);

    // rule 12:
    ruleTable[65] = 12;

    // rule 13:
    temp = {160, 161, 176, 177};
    addToTable(&temp, ruleTable, 13);

    // rule 14:
    temp = {37, 41, 45, 53, 57, 61, 101, 105, 109, 117, 121, 125};
    addToTable(&temp, ruleTable, 14);

    // rule 15:
    temp = {69, 73, 77, 81, 85, 89, 93};
    addToTable(&temp, ruleTable, 15);

    // rule 16:
    temp = {162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172,
           173, 174, 175, 178, 179, 180, 181, 182, 183, 184, 185,
           186, 187, 188, 189, 190, 191, 224, 225, 226, 227, 228,
           229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
           240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250,
           251, 252, 253, 254, 255};
    addToTable(&temp, ruleTable, 16);
}

// Implements the [] operator for maps
// because it doesn't work for some reason (why)
vectorLine* get_replace(Vec2i key, vectorLine* value, map<Vec2i, vectorLine*, vec2i_compare>* hashmap)
{
    auto it = hashmap->find(key);

    // found item: return iterator to current mapping
    if (it != hashmap->end())
      return (*it).second;

    // didn't find item: insert key and mapping
    // is there a way to just not replace it if it's there?
    else
    {
        hashmap->insert(make_pair(key, value));
        return (*it).second;
    }
}

// Two lines end in the same spot, so create a node and
// assign the node as both of the lines' endpoints.
void closeMultiple (set<vectorLine*>* lines, map<Vec2i, vectorLine*, vec2i_compare>* pxToLine,
                      map<Vec2i, vectorLine*, vec2i_compare>* nodeToLine, Vec2i cur, Vec2i end1, Vec2i end2)
{
    makeNode(lines, pxToLine, nodeToLine, cur);
    closeSingle(lines, pxToLine, nodeToLine, cur, end1);
    closeSingle(lines, pxToLine, nodeToLine, cur, end2);
}

// Two lines meet in a non-extreme point and thus shouldn't share a node.
// Connects the two lines by letting the first line "absorb" the second's pixels
// (and endpoint) and deleting the second line from the line set.
void connectLines(set<vectorLine*>* lines, map<Vec2i, vectorLine*, vec2i_compare>* pxToLine,
                  map<Vec2i, vectorLine*, vec2i_compare>* nodeToLine, Vec2i cur, Vec2i conn1, Vec2i conn2)
{
    vectorLine* line1;
    vectorLine* line2;

    // if both points are nodes:
    // create a new line that uses them as start
    // and end points and which has exactly one pixel.
    if(isNode(conn1, pxToLine) && isNode(conn2, pxToLine))
    {
        // get lines
        line1 = get_replace(conn1, line1, nodeToLine);
        line2 = get_replace(conn2, line2, nodeToLine);

        // create new link line
        vectorLine* new_line = makeNode(lines, pxToLine, nodeToLine, conn1);
        new_line->setEnd(conn2);
        new_line->addPixels(cur);

        get_replace(conn2, new_line, nodeToLine);
        get_replace(cur, new_line, pxToLine);
        lines->insert(new_line);

        // delete separately because erase only calls "direct" destructors
        //delete line1;
        //delete line2;

        // delete old lines
        lines->erase(line1);
        lines->erase(line2);
    }

    // only first point is a node:
    // Set node as end point for line of second point
    else if (isNode(conn1, pxToLine))
    {
        line2 = get_replace(conn2, line2, pxToLine);
        line2->setEnd(conn1);

        get_replace(conn1, line2, nodeToLine);
        get_replace(cur, line2, pxToLine);
        line2->addPixels(cur);

        //delete line1;
        lines->erase(line1);
    }

    // only second point is a node
    // Set node as end point for line of first point
    else if (isNode(conn2, pxToLine))
    {
        line1 = get_replace(conn1, line1, pxToLine);
        line1->setEnd(conn2);

        get_replace(conn2, line1, nodeToLine);
        get_replace(cur, line1, pxToLine);
        line2->addPixels(cur);

        //delete line2;
        lines->erase(line2);
    }

    // Both points belong to valid lines
    // Let first line absorb the second one.
    else
    {
        line1 = get_replace(conn1, line1, pxToLine);
        line2 = get_replace(conn2, line2, pxToLine);

        // update pixel to line mapping
        for(auto px = line2->getPixels().begin(); px != line2->getPixels().end(); px++)
        {
            line1->addPixels(*px);
            get_replace(*px, line1, pxToLine);
        }

        // set endpoint of first line to startpoint of second line
        line1->setEnd(conn2);
        get_replace(conn2, line1, nodeToLine);

        // delete obsolete second line from line set
        //delete line2;
        lines->erase(line2);
    }
}

// Add the current pixel to the line "otherPx"
// belongs to.
void extendLine(set<vectorLine*>* lines, map<Vec2i, vectorLine*, vec2i_compare>* pxToLine,
                map<Vec2i, vectorLine*, vec2i_compare>* nodeToLine, Vec2i cur, Vec2i otherPx)
{
    vectorLine* extend = NULL;

    if(isNode(otherPx, pxToLine))
        extend = get_replace(otherPx, extend, nodeToLine);

    else
        extend = get_replace(otherPx, extend, pxToLine);

    // Extend line by current pixel
    extend->addPixels(cur);
    get_replace(cur, extend, pxToLine);
}

// One line ends, another begins. Assign the current pixel as ending point of the old line.
void closeSingle(set<vectorLine*>* lines, map<Vec2i, vectorLine*, vec2i_compare>* pxToLine,
                 map<Vec2i, vectorLine*, vec2i_compare>* nodeToLine, Vec2i cur, Vec2i otherPx)
{
    vectorLine* line = NULL;

    // close old line
    if(isNode(otherPx, pxToLine))
        line = get_replace(otherPx, line, nodeToLine);

    else
        line = get_replace(otherPx, line, pxToLine);

    line->setEnd(cur);

    // add "path" identification here?
    // are polygons always CCW? How to recover topology? For
    // cycle walking to work, the vectors should point in the
    // same direction...
    // aren't the resulting graphs DAGs by default..?
    // Top to bottom walks only! Also, what about 3-junctions?



}

// Create a new node as the start point of a vector.
vectorLine* makeNode(set<vectorLine*>* lines, map<Vec2i, vectorLine*, vec2i_compare>* pxToLine,
                map<Vec2i, vectorLine*, vec2i_compare>* nodeToLine, Vec2i cur)
{
    vectorLine* new_line = new vectorLine();

    new_line->setStart(cur);
    get_replace(cur, new_line, nodeToLine);
    lines->insert(new_line);
    pxToLine->erase(cur); // remove "normal pixel" status

    return new_line;
}

// Check if cur is a node.
bool isNode(Vec2i cur, map<Vec2i, vectorLine*, vec2i_compare>* pxToLine)
{
    return (pxToLine->find(cur) == pxToLine->end());
}

// Applies the vectorizing rule that corresponds
// to the neighborhood situation encoded by the
// 8-bit value "nBits".
// The array ruleTable provides a lookup for each
// of the 256 possible neighborhood states by listing
// the rule that applies to each state.; // map each black pixel to a line
void applyRule(Vec2i cur, uint8_t nBits, int* ruleTable, map<Vec2i, vectorLine*, vec2i_compare>* pxToLine,
               map<Vec2i, vectorLine*, vec2i_compare>* nodeToLine, set<vectorLine*>* lines)
{
    // get rule from table
    int rule = ruleTable[nBits];

    // apply fitting rule
    switch (rule)
    {
    case 1:
    {
        // do nothing
        break;
    }

    case 2:
    {
        // make a node: node is starting point of only one line
        makeNode(lines, pxToLine, nodeToLine, cur);
        break;
    }

    case 3:
    {
        // make a node
        // and set end point of NE pixel line to node:
        // Here, one node is both the ending point an starting point
        // of two lines respectively.
        closeSingle(lines, pxToLine, nodeToLine, cur, Vec2i(cur[0]-1, cur[1]+1));
        makeNode(lines, pxToLine, nodeToLine, cur);
        break;
    }

    case 4:
    {
        // make a node
        // and set end point of W pixel line to node
        closeSingle(lines, pxToLine, nodeToLine, cur, Vec2i(cur[0], cur[1]-1));
        makeNode(lines, pxToLine, nodeToLine, cur);
        break;
    }

    case 5:
    {
        // make a node
        // and set end point of NW pixel line to node
        closeSingle(lines, pxToLine, nodeToLine, cur, Vec2i(cur[0]-1, cur[1]-1));
        makeNode(lines, pxToLine, nodeToLine, cur);
        break;
    }

    case 6:
    {
        // make a node
        // and set end point of N pixel line to node
        closeSingle(lines, pxToLine, nodeToLine, cur, Vec2i(cur[0]-1, cur[1]));
        makeNode(lines, pxToLine, nodeToLine, cur);
        break;
    }

    case 7:
    {
        // Extend line of NE pixel: No local extreme, so the previous
        // line simply continues.
        extendLine(lines, pxToLine, nodeToLine, cur, Vec2i(cur[0]-1, cur[1]+1));
        break;
    }

    case 8:
    {
        // Extend line of W pixel
        extendLine(lines, pxToLine, nodeToLine, cur,  Vec2i(cur[0], cur[1]-1));
        break;
    }

    case 9:
    {
        // Extend line of NW pixel
        extendLine(lines, pxToLine, nodeToLine, cur,  Vec2i(cur[0]-1, cur[1]-1));
        break;
    }

    case 10:
    {
        // Extend line of N pixel
        extendLine(lines, pxToLine, nodeToLine, cur,  Vec2i(cur[0]-1, cur[1]));
        break;
    }

    case 11:
    {
        // Connect line of W pixel
        // with line of NE pixel: Two lines meet, but the meeting
        // point is not an extreme; delete one line and extend the
        // other to cover all its pixels and assume its starting point
        // as its ending point.
        connectLines(lines, pxToLine, nodeToLine, Vec2i(cur[0], cur[1]-1), cur, Vec2i(cur[0]-1, cur[1]+1));
        break;
    }

    case 12:
    {
        // Connect line of of NW pixel
        // with line of NE pixel
        connectLines(lines, pxToLine, nodeToLine, Vec2i(cur[0]-1, cur[1]-1), cur, Vec2i(cur[0]-1, cur[1]+1));
        break;
    }

    case 13:
    {
        // Connect line of W pixel
        // with line of N pixel
        connectLines(lines, pxToLine, nodeToLine, Vec2i(cur[0], cur[1]-1), cur, Vec2i(cur[0]-1, cur[1]));
        break;
    }

    case 14:
    {
        // Make a node
        // and set end points of W pixel line
        // AND of NE pixel line to the node
        closeMultiple(lines, pxToLine, nodeToLine, cur, Vec2i(cur[0], cur[1]-1), Vec2i(cur[0]-1, cur[1]+1));
        break;
    }

    case 15:
    {
        // Make a node
        // and set end points of NW pixel line
        // AND of NE pixel line to the node
        closeMultiple(lines, pxToLine, nodeToLine, cur, Vec2i(cur[0]-1, cur[1]-1), Vec2i(cur[0]-1, cur[1]+1));
        break;
    }

    case 16:
    {
        // Make a node
        // and set end points of W pixel line
        // AND of N pixel line to the node
        closeMultiple(lines, pxToLine, nodeToLine, cur, Vec2i(cur[0], cur[1]-1), Vec2i(cur[0]-1, cur[1]));
        break;
    }

    default:
    {
        // Rule not specified - error
        cout << "Rule " << rule << " was not specified!\n";
        assert(rule > 0 && rule < 17);
    }
    }

}


// Expects a thinned binary image (black = line pixels, white = background).
//
// Use a pixel walking technique to find vector lines.
// This technique is due to:
//
// Moore L. R.: Software for cartographic raster-to-vector conversion.
// International archives of photogrammetry and remote sensing.
// Volume XXIX. Part B4. Comission IV. Washington, D.C. 1992
//
// Also contains additional postprocessing of the vectors by means of
// the Douglas-Pecker-Rahmer algorithm for simplifying lines and
// restoring topology.
set<vectorLine*> mooreVector(Mat image)
{
    int ruleTable[256];
    initRuleTable(ruleTable);

    set<vectorLine*> lines; // all found lines
    map<Vec2i, vectorLine*, vec2i_compare> pxToLine; // map each black pixel to a line
    map<Vec2i, vectorLine*, vec2i_compare> nodeToLine; // map each endpoint to the line it belongs to
    vector<vector<vectorLine>> paths; // connected vector lines: lines that share a node

    // transform matrix points one by one
    // by applying rule according to neighborhood
    for(int i = 0; i < image.rows; i++)
    {
        for(int j = 0; j < image.cols; j++)
        {
            // look only at black pixels
            if(image.at<uchar>(i, j) == 0)
            {
                // get neighborhood encoding of current pixel
                uint8_t nbh = encodeNeighbors(&image, Vec2i(i, j));

                // apply neighborhood rule
                applyRule(Vec2i(i, j), nbh, ruleTable, &pxToLine, &nodeToLine, &lines);
            }
        }
    }

    return lines;
}


// Create a .svg file "filename.svg" based on the extracted vector lines.
void vectorsToFile (Mat* image, set<vectorLine*> lines, string filename)
{
    // debug: overlay found lines on image in blue
    cvtColor(*image, *image, CV_GRAY2RGB);
    Mat vectoronly = Mat(image->rows, image->cols, image->type());

    for(auto l = lines.begin(); l != lines.end(); l++)
    {
        Vec2i start = (*l)->getStart();
        Vec2i end = (*l)->getEnd();

        Point pt1 = Point(start[1], start[0]);
        Point pt2 = Point(end[1], end[0]);

        line(vectoronly, pt1, pt2, Scalar(255, 0, 0), 1);
    }

    namedWindow("VECTORIZED", WINDOW_NORMAL);
    imshow("VECTORIZED", vectoronly);

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
        Vec2i start = (*l)->getStart();
        Vec2i end = (*l)->getEnd();

        cairo_move_to(cr, start[1], start[0]);
        cairo_line_to(cr, end[1], end[0]);
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

    set<vectorLine*> lines;
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
    lines = mooreVector(thinned);

    // refine vectors by removing unnecessary nodes
    cout << "Refining vector data... \n";
    // refineVectors(lines);

    // recover image topology and add color
    cout << "Recovering image topology... \n";
    // recoverTopology(lines);

    // write vector lines to file
    cout << "Writing vector data to file << " << filename << ".svg...\n";
    vectorsToFile(&thinned, lines, filename);

    // delete allocated line objects
    cout << "Cleaning up... \n";
    for(auto l = lines.begin(); l != lines.end(); l++)
        delete (*l);

    cout << "VECTORIZATION DONE!\n";
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
void refineVectors (std::set<vectorLine *> *lines)
{
    // R-Tree for starting points: int IDs (can't handle Vec2f - maybe array?), float values, 2 dimensions.
    /*RTree<int, float, 2, float> start_tree;
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
            cairo_move_to(cr, arg.input[2], arg.input[3]);
            cairo_line_to(cr, arg.found[0], arg.found[1]);
            cairo_stroke(cr);
        }
    }


    // check if the next line's starting point
    // is close the the current endpoint; If yes,
    // replace the endpoint of the current line by
    // the next line's starting point.
    if((l+1) != lines.end())
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
