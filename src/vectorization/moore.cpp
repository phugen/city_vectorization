#include "include/vectorization/vectorize.hpp"

#include <map>
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
uint8_t encodeNeighbors (Mat* image, pixel* curPixel)
{
    int rows = image->rows;
    int cols = image->cols;
    int i = curPixel->coord[0]; // y-coordinate
    int j = curPixel->coord[1]; // x-coordinate
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
// Each neighborhood is expected to has exactly ONE rule mapped to it!
void initRuleTable(int* ruleTable)
{
    // rule 1:
    // (do nothing)
    ruleTable[0] = 1;

    // -----------------------------------------

    // rule 2:
    // (make a node)
    vector<int> temp = {2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18,
                        19, 20, 22, 23, 24, 26, 27, 28, 30, 31};
    addToTable(&temp, ruleTable, 2);


    // ------- closeSingle rules ---------------
    // rule 3:
    // (Make a node, close line of NE pixel)
    ruleTable[1] = 3;
    ruleTable[21] = 3;

    // rule 4:
    // (Make a node, close line of W pixel)
    temp = {32, 42, 43, 46, 47, 48, 56, 58, 59, 60, 62, 63, 96, 106,
            107, 110, 111, 112, 120, 122, 123, 124, 126, 127};
    addToTable(&temp, ruleTable, 4);

    // rule 5:
    // (Make a node, close line of NW pixel)
    temp = {64, 66, 67, 70, 71, 72, 74, 75, 76, 78, 79, 82, 83, 84, 86, 87, 88, 90, 91, 92,
            94, 95};
    addToTable(&temp, ruleTable, 5);

    // rule 6:
    // (Make a node, close line of N pixel)
    temp = {128, 129, 131, 135, 138, 139, 142, 143, 144, 146, 147, 148,
            149, 150, 151, 154, 155, 158, 159, 192, 193, 195, 199,
            202, 203, 206, 207, 210, 211, 212, 213, 214, 215, 218,
            219, 222, 223, 200};
    addToTable(&temp, ruleTable, 6);


    // ------------ extend rules --------------
    // rule 7:
    // (Extend line of NE pixel)
    temp = {5, 17, 25, 29};
    addToTable(&temp, ruleTable, 7);

    // rule 8:
    // (Extend line of W pixel)
    temp = {34, 35, 36, 38, 39, 40, 44, 50, 51, 52, 54, 55, 98, 99,
            100, 102, 103, 104, 108, 114, 115, 116, 118, 119};
    addToTable(&temp, ruleTable, 8);

    // rule 9:
    // (Extend line of NW pixel)
    temp = {68, 80};
    addToTable(&temp, ruleTable, 9);

    // rule 10:
    // (Extend line of N pixel)
    temp = {130, 132, 133, 134, 136, 137, 140, 141, 145, 152,
            153, 156, 157, 194, 196, 197, 198, 200, 201, 204, 205,
            208, 209, 216, 217, 220, 221};
    addToTable(&temp, ruleTable, 10);


    // ------- connect rules ---------------

    // CONNECT rules are CLOSEMULTIPLE rules without a following rule2_makeNode!

    // rule 11:
    // (Connect line of W pixel with line of NE pixel)
    temp = {33, 49, 97, 113};
    addToTable(&temp, ruleTable, 11);

    // rule 12:
    // (Connect line of NW pixel with line of NE pixel)
    ruleTable[65] = 12;

    // rule 13:
    // (Connect line of W pixel with line of N pixel)
    temp = {160, 161, 176, 177};
    addToTable(&temp, ruleTable, 13);

    // ------- closeMultiple rules -----------
    // rule 14:
    // (Make a node, close line of W pixel, close line of NE pixel)
    temp = {37, 41, 45, 53, 57, 61, 101, 105, 109, 117, 121, 125};
    addToTable(&temp, ruleTable, 14);

    // rule 15:
    // (Make a node, close line of NW pixel, close line of NE pixel)
    temp = {69, 73, 77, 81, 85, 89, 93};
    addToTable(&temp, ruleTable, 15);

    // rule 16:
    // (Make a node, close line of W pixel, close line of N pixel)
    temp = {162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172,
            173, 174, 175, 178, 179, 180, 181, 182, 183, 184, 185,
            186, 187, 188, 189, 190, 191, 224, 225, 226, 227, 228,
            229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
            240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250,
            251, 252, 253, 254, 255};
    addToTable(&temp, ruleTable, 16);
}


// Create a new node as the start point of a vector.
vectorLine* rule2_makeNode(set<vectorLine*>* lines, pixel* cur)
{
    vectorLine* new_line = new vectorLine();
    new_line->setStart(cur);
    new_line->setEnd(cur);
    lines->insert(new_line);

    cur->isNode = true;
    cur->line = new_line;

    return new_line;
}

void rule16_closeMultW_N(set<vectorLine*>* lines, pixel* cur, pixel* west, pixel* north, pixel* northWest, pixel* northEast)
{
    // NORTHWEST STATES?


    // north line was closed already
    // so close only west line
    if((northEast->line != NULL) &&
       (northEast->coord != Vec2i(-1, -1))) // check if out of bounds - count as white pixel
    {
        west->line->setEnd(west);

        if(west != west->line->getStart())
            west->isNode = false;

        rule2_makeNode(lines, cur);
    }

    // normal vertical line, closeMult as usual
    else
    {
        // junction case in which west didn't have a line
        if(west->line == NULL)
        {
            rule2_makeNode(lines, west);
        }

        // junction case in which north didn't have a line
        if(north->line == NULL)
        {
            rule2_makeNode(lines, north);
        }

        west->line->setEnd(west);
        north->line->setEnd(north);

        if(west != west->line->getStart())
            west->isNode = false;

        if(north != north->line->getStart())
            north->isNode = false;

        rule2_makeNode(lines, cur);
    }

    // add "path" identification here?
    // are polygons always CCW? How to recover topology? For
    // cycle walking to work, the vectors should point in the
    // same direction...
    // aren't the resulting graphs DAGs by default..?
    // Top to bottom walks only! Also, what about 3-junctions?
}


void rule15_closeMultNW_NE(set<vectorLine*>* lines, pixel* cur, pixel* northWest, pixel* northEast)
{
    northWest->isNode = true;
    northWest->line->setEnd(northWest);

    northEast->isNode = true;
    northEast->line->setEnd(northEast);

    rule2_makeNode(lines, cur);
}


void rule14_closeMultW_NE(set<vectorLine*>* lines, pixel* cur, pixel* west, pixel* northEast)
{
    west->isNode = true;
    west->line->setEnd(west);

    //northEast->isNode = true;
    //northEast->line->setEnd(northEast);

    rule2_makeNode(lines, cur);
}

void rule13_connectW_N(set<vectorLine*>* lines, pixel* cur, pixel* west, pixel* north, pixel* northEast)
{
    // north is beginning of a line
    // so only close W
    if(northEast->line != NULL)
    {
        cur->isNode = true;
        west->line->setEnd(west);

        if(west != west->line->getStart())
            west->isNode = false;

        rule2_makeNode(lines, cur);
    }

    // north is normal pixel
    // so connect W with N
    else
    {
        cur->isNode = true;
        cur->line = west->line;
        west->line->setEnd(cur);
        north->line->setEnd(north);

        if(west != west->line->getStart())
            west->isNode = false;

        north->isNode = true;
    }
}


void rule12_connectNW_NE(pixel* cur, pixel* northWest, pixel* northEast)
{
    cur->isNode = true;
    northWest->line->setEnd(cur);
    northEast->line->setEnd(cur);

    if(northWest != northWest->line->getStart())
        northWest->isNode = false;

    if(northEast != northEast->line->getStart())
        northEast->isNode = false;

    cur->line = northWest->line;
}

void rule11_connectW_NE(set<vectorLine*>* lines, pixel* cur, pixel* west, pixel* northEast)
{
    // set end of joining lines to cur
    cur->isNode = true;
    west->line->setEnd(cur);

    // revoke node status (probably not needed)
    if(west != west->line->getStart())
        west->isNode = false;

    cur->line = west->line;
}

void rule10_extendN(set<vectorLine*>* lines, pixel* cur, pixel* north, pixel* northEast, pixel* northWest)
{
    // North is the start of a vector,
    // create new line at cur
    if(northEast->line != NULL)
    {
        rule2_makeNode(lines, cur);
    }

    // End of horizontal vector,
    // close it and create new vector at cur
    else if((north == north->line->getEnd()) &&
       (northWest->line != NULL))
    {
        north->line->setEnd(north);
        rule2_makeNode(lines, cur);
    }

    // no special case - just add cur to north line
    else
    {
        // don't push end point! - needed for connectW_N
        //???
        cur->isNode = true;
        north->line->setEnd(cur);
        cur->line = north->line;

        if(north != north->line->getStart())
            north->isNode = false;
    }
}

void rule9_extendNW(pixel* cur, pixel* northWest)
{
    cur->isNode = true;
    northWest->line->setEnd(cur);
    cur->line = northWest->line;

    // if west isn't the first point
    // in the line, revoke its node status
    if(northWest != northWest->line->getStart())
        northWest->isNode = false;
}

void rule8_extendW(set<vectorLine*>* lines, pixel* cur, pixel* west, pixel* northWest)
{
    // West is the end of a vector, close it and
    // start new line at cur
    if(west == west->line->getEnd() &&
       northWest->line != NULL)
    {
        west->line->setEnd(west);
        rule2_makeNode(lines, cur);
    }

    // west is part of the same line as cur,
    // so just add cur to the line
    else
    {
        // push end pointer
        cur->isNode = true;
        west->line->setEnd(cur);
        cur->line = west->line;

        // if west isn't the first point
        // in the line, revoke its node status
        if(west != west->line->getStart())
            west->isNode = false;
    }
}

void rule7_extendNE(pixel* cur, pixel* northEast)
{
    // set current point as new start point
    cur->isNode = true;
    northEast->line->setStart(cur);
    cur->line = northEast->line;
    northEast->isNode = false;
}

void rule6_closeN(set<vectorLine*>* lines, pixel* cur, pixel* north, pixel* northWest, pixel* northEast)
{
    // north is start of a line; don't close it but
    // instead begin new line at cur
    if((north == north->line->getStart()) &&
      (northEast->line != NULL))
    {
        rule2_makeNode(lines, cur);
    }

    // north is the end of a line; close it
    // normally and make a node at cur
    else
    {
        north->isNode = true;
        north->line->setEnd(north);
        rule2_makeNode(lines, cur);
    }
}

void rule5_closeNW(set<vectorLine*>* lines, pixel* cur, pixel* northWest)
{
    northWest->isNode = true;
    northWest->line->setEnd(northWest);

    rule2_makeNode(lines, cur);
}

void rule4_closeW(set<vectorLine*>* lines, pixel* cur, pixel* west)
{
    west->isNode = true;
    cur->isNode = true;

    rule2_makeNode(lines, cur);
}

void rule3_closeNE(set<vectorLine*>* lines, pixel* cur, pixel* northEast)
{
    // lines "comes" from the right
    // so set start instead of end
    northEast->isNode = true;
    northEast->line->setStart(cur);

    //rule2_makeNode(lines, cur);
}


// Assign non-dummy values to neighbor pixels based
// on whether the current pixel is in a corner etc.
void assignNeighborPointers(Vec2i coord, Mat* image, pixel** northEast, pixel** north, pixel** northWest, pixel** west, vector<pixel*>* pixels)
{
    // in top left corner
    if((coord[0] == 0) && (coord[1] == 0))
    {
        // none
    }

    // in top right corner
    else if((coord[0]) == 0 && (coord[1] == (image->cols - 1)))
    {
        *west = (pixels->at(coord[0] * image->cols + (coord[1]-1)));
    }

    // in bottom left corner
    else if(((coord[0]) == (image->rows - 1)) && (coord[1] == 0))
    {
        *north = (pixels->at((coord[0]-1) * image->cols + coord[1]));
        *northEast = (pixels->at((coord[0]-1) * image->cols + (coord[1]+1)));
    }

    // in bottom right corner
    else if(((coord[0]) == (image->rows - 1)) && (coord[1] == (image->cols - 1)))
    {
        *west = (pixels->at(coord[0] * image->cols + (coord[1]-1)));
        *northWest = (pixels->at((coord[0]-1) * image->cols + (coord[1]-1)));
        *north = (pixels->at((coord[0]-1) * image->cols + coord[1]));
    }

    // top border
    else if(coord[0] == 0)
    {
        *west = (pixels->at(coord[0] * image->cols + (coord[1]-1)));
    }

    // left border
    else if(coord[1] == 0)
    {
        *north = (pixels->at((coord[0]-1) * image->cols + coord[1]));
        *northEast = (pixels->at((coord[0]-1) * image->cols + (coord[1]+1)));
    }

    // right border
    else if(coord[1] == (image->cols - 1))
    {
        *west = (pixels->at(coord[0] * image->cols + (coord[1]-1)));
        *northWest = (pixels->at((coord[0]-1) * image->cols + (coord[1]-1)));
        *north = (pixels->at((coord[0]-1) * image->cols + coord[1]));
    }

    // anywhere else in the image
    else
    {
        *west = (pixels->at(coord[0] * image->cols + (coord[1]-1)));
        *northWest = (pixels->at((coord[0]-1) * image->cols + (coord[1]-1)));
        *north = (pixels->at((coord[0]-1) * image->cols + coord[1]));
        *northEast = (pixels->at((coord[0]-1) * image->cols + (coord[1]+1)));
    }
}


// Applies the vectorizing rule that corresponds
// to the neighborhood situation encoded by the
// 8-bit value "nBits".
// The array ruleTable provides a lookup for each
// of the 256 possible neighborhood states by listing
// the rule that applies to each state.; // map each black pixel to a line
void applyRule(Mat* image, pixel* cur, uint8_t nBits, int* ruleTable, set<vectorLine*>* lines, vector<pixel*>* pixels, pixel* dummy)
{

    Vec2i coord = cur->coord;

    pixel* northEast = dummy;
    pixel* north = dummy;
    pixel* northWest = dummy;
    pixel* west = dummy;

    // predefine directional pixels based on current pixel
    assignNeighborPointers(coord, image, &northEast, &north, &northWest, &west, pixels);

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
            rule2_makeNode(lines, cur);
            break;
        }

        case 3:
        {
            // make a node
            // and set end point of NE pixel line to node:
            // Here, one node is both the ending point an starting point
            // of two lines respectively.
            rule3_closeNE(lines, cur, northEast);
            break;
        }

        case 4:
        {
            // make a node
            // and set end point of W pixel line to node
            rule4_closeW(lines, cur, west);
            break;
        }

        case 5:
        {
            // make a node
            // and set end point of NW pixel line to node
            rule5_closeNW(lines, cur, northWest);
            break;
        }

        case 6:
        {
            // make a node
            // and set end point of N pixel line to node
            rule6_closeN(lines, cur, north, northWest, northEast);
            break;
        }

        case 7:
        {
            // Extend line of NE pixel: No local extreme, so the previous
            // line simply continues.
            rule7_extendNE(cur, northEast);
            break;
        }

        case 8:
        {
            // Extend line of W pixel
            rule8_extendW(lines, cur, west, northWest);
            break;
        }

        case 9:
        {
            // Extend line of NW pixel
            rule9_extendNW(cur, northWest);
            break;
        }

        case 10:
        {
            // Extend line of N pixel
            rule10_extendN(lines, cur, north, northEast, northWest);
            break;
        }

        case 11:
        {
            // Connect line of W pixel
            // with line of NE pixel: Two lines meet in one point.
            // Set both their endpoints to this point.
            rule11_connectW_NE(lines, cur, west, northEast);
            break;
        }

        case 12:
        {
            // Connect line of of NW pixel
            // with line of NE pixel
            rule12_connectNW_NE(cur, northWest, northEast);
            break;
        }

        case 13:
        {
            // Connect line of W pixel
            // with line of N pixel
            rule13_connectW_N(lines, cur, west, north, northEast);
            break;
        }

        case 14:
        {
            // Make a node
            // and set end points of W pixel line
            // AND of NE pixel line to the node
            rule14_closeMultW_NE(lines, cur, west, northEast);
            break;
        }

        case 15:
        {
            // Make a node
            // and set end points of NW pixel line
            // AND of NE pixel line to the node
            rule15_closeMultNW_NE(lines, cur, northWest, northEast);
            break;
        }

        case 16:
        {
            // Make a node
            // and set end points of W pixel line
            // AND of N pixel line to the node
            rule16_closeMultW_N(lines, cur, west, north, northWest, northEast);
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


// debug function: outputs binary encoding
// of a neighborhood state as an image.
void stateToImage (Mat image, int nbh)
{
    int nbh_orig = nbh;

    int rowstep = image.rows / 3;
    int colstep = image.cols / 3;

    int maxcode = 128;
    int x, y;

    x = colstep;
    y = rowstep;

    // fill image with white color
    rectangle(image, Rect(0, 0, image.cols, image.rows), Scalar(255, 255, 255), CV_FILLED);

    // draw center
    rectangle(image, Rect(x, y, colstep, rowstep), Scalar(0, 0, 255), CV_FILLED);

    for (maxcode = 128; nbh > 0; maxcode /= 2)
    {
        if(nbh - 128 >= 0 && maxcode == 128)
        {
            nbh -= 128;
            x = colstep;
            y = 0;
        }

        else if (nbh - 64 >= 0 && maxcode == 64)
        {
            nbh -= 64;
            x = 0;
            y = 0;
        }

        else if (nbh - 32 >= 0 && maxcode == 32)
        {
            nbh -= 32;
            x = 0;
            y = rowstep;
        }

        else if (nbh - 16 >= 0 && maxcode == 16)
        {
            nbh -= 16;
            x = 0;
            y = 2 * rowstep;
        }

        else if (nbh - 8 >= 0 && maxcode == 8)
        {
            nbh -= 8;
            x = colstep;
            y = 2 * rowstep;
        }

        else if (nbh - 4 >= 0 && maxcode == 4)
        {
            nbh -= 4;
            x = 2 * colstep;
            y = 2 * rowstep;
        }

        else if (nbh - 2 >= 0 && maxcode == 2)
        {
            nbh -= 2;
            x = 2 * colstep;
            y = rowstep;
        }

        else if (nbh - 1 >= 0 && maxcode == 1)
        {
            nbh -= 1;
            x = 2 * colstep;
            y = 0;
        }

        rectangle(image, Rect(x, y, colstep, rowstep), Scalar(0, 0, 0), CV_FILLED);
    }

    // draw cell separation lines
    line(image, Point(colstep, 0), Point(colstep, 3 * rowstep), Scalar(255, 0, 0), 3);
    line(image, Point(2 * colstep, 0), Point(2 * colstep, 3 * rowstep), Scalar(255, 0, 0), 3);

    line(image, Point(0, rowstep), Point(3 * colstep, rowstep), Scalar(255, 0, 0), 3);
    line(image, Point(0, 2 * rowstep), Point(3 * colstep, 2 * rowstep), Scalar(255, 0, 0), 3);


    string number = std::to_string(nbh_orig);
    string filename = number + string(".png");
    imwrite(filename.c_str(), image);
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
map<pixel*, vectorLine*> mooreVector(Mat image, vector<pixel*>* pixels, pixel* dummy)
{
    /*Mat test = Mat(300, 300, CV_8UC1);
    cvtColor(test, test, CV_GRAY2BGR);
    stateToImage(test, (uint8_t) 50);

    for(int i = 1; i < 256; i++)
    {
        stateToImage(test, i);
    }

    waitKey(0);*/

    int ruleTable[256]; // links each neighborhood encoding to a rule
    initRuleTable(ruleTable);


    set<vectorLine*> lines; // all found vector lines

    // transform matrix points one by one
    // by applying rule according to neighborhood
    for(int i = 0; i < image.rows; i++)
    {
        for(int j = 0; j < image.cols; j++)
        {
            // look only at black pixels
            if(image.at<uchar>(i, j) == 0)
            {
                // get current pixel pointer
                int64_t addr = i * image.cols + j;

                pixel* cur = pixels->at(addr);

                // get neighborhood encoding of current pixel
                uint8_t nbh = encodeNeighbors(&image, cur);

                // apply neighborhood rule
                applyRule(&image, cur, nbh, ruleTable, &lines, pixels, dummy);
            }
        }
    }

    delete dummy;

    // close dangling lines by setting endpoint to startpoint
    for(auto l = lines.begin(); l != lines.end(); l++)
    {
        if((*l)->getEnd() == NULL)
        {
            (*l)->setEnd((*l)->getStart());
        }
    }

    // build node->line mapping
    map<pixel*, vectorLine*> nodeToLine;

    for(auto l = lines.begin(); l != lines.end(); l++)
    {
        nodeToLine.insert(make_pair((*l)->getStart(), *l));
        nodeToLine.insert(make_pair((*l)->getEnd(), *l));
    }

    return nodeToLine;
}
