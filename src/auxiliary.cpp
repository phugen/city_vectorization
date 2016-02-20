/**
  * Various helper functions are defined here, such as finding all
  * eight-connected, black pixels around a specified pixel.
  *
  * Author: Phugen
  */

#include "include/auxiliary.hpp"
#include "include/colorconversions.hpp"
#include "include/connectedcomponent.hpp"

#include <stack>
#include <list>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace cv;


// Returns a list of all black eight-connected neighbors
// of the input pixel.
//
// DON'T mistake this for the check in unionfindcomponents.cpp, which
// operates in a scanline fashion, and thus
// gives different neighbors for a pixel.
vector<Vec2i> eightConnectedBlackNeighbors(Vec2i pixel, Mat* image)
{
    vector<Vec2i> neighbors; // output list
    int rows = image->rows;
    int cols = image->cols;

    int i = pixel[0]; // rows
    int j = pixel[1]; // cols


    // top left pixel
    if(i == 0 && j == 0)
    {
        if(image->at<uchar>(i+1, j) == 0) neighbors.push_back(Vec2i(i+1, j));
        if(image->at<uchar>(i+1, j+1) == 0) neighbors.push_back(Vec2i(i+1, j+1));
        if(image->at<uchar>(i, j+1) == 0) neighbors.push_back(Vec2i(i, j+1));
    }

    // top right pixel
    else if(i == 0 && j == (cols - 1))
    {
        if(image->at<uchar>(i+1, j) == 0) neighbors.push_back(Vec2i(i+1, j));
        if(image->at<uchar>(i+1, j-1) == 0) neighbors.push_back(Vec2i(i+1, j-1));
        if(image->at<uchar>(i, j-1) == 0) neighbors.push_back(Vec2i(i, j-1));
    }


    // bottom left pixel
    else if (i == (rows - 1) && j == 0)
    {
        if(image->at<uchar>(i, j+1) == 0) neighbors.push_back(Vec2i(i, j+1));
        if(image->at<uchar>(i-1, j+1) == 0) neighbors.push_back(Vec2i(i-1, j+1));
        if(image->at<uchar>(i-1, j) == 0) neighbors.push_back(Vec2i(i-1, j));
    }

    // bottom right pixel
    else if (i == (rows - 1) && j == (cols - 1))
    {
        if(image->at<uchar>(i-1, j) == 0) neighbors.push_back(Vec2i(i-1, j));
        if(image->at<uchar>(i-1, j-1) == 0) neighbors.push_back(Vec2i(i-1, j-1));
        if(image->at<uchar>(i, j-1) == 0) neighbors.push_back(Vec2i(i, j-1));
    }

    // upper border
    else if (i == 0)
    {
        if(image->at<uchar>(i, j+1) == 0) neighbors.push_back(Vec2i(i, j+1));
        if(image->at<uchar>(i+1, j+1) == 0) neighbors.push_back(Vec2i(i+1, j+1));
        if(image->at<uchar>(i+1, j) == 0) neighbors.push_back(Vec2i(i+1, j));
        if(image->at<uchar>(i+1, j-1) == 0) neighbors.push_back(Vec2i(i+1, j-1));
        if(image->at<uchar>(i, j-1) == 0) neighbors.push_back(Vec2i(i, j-1));
    }

    // left border
    else if (j == 0)
    {
        if(image->at<uchar>(i-1, j) == 0) neighbors.push_back(Vec2i(i-1, j));
        if(image->at<uchar>(i-1, j+1) == 0) neighbors.push_back(Vec2i(i-1, j+1));
        if(image->at<uchar>(i, j+1) == 0) neighbors.push_back(Vec2i(i, j+1));
        if(image->at<uchar>(i+1, j+1) == 0) neighbors.push_back(Vec2i(i+1, j+1));
        if(image->at<uchar>(i+1, j) == 0) neighbors.push_back(Vec2i(i+1, j));
    }

    // right border
    else if (j == (cols - 1))
    {
        if(image->at<uchar>(i-1, j) == 0) neighbors.push_back(Vec2i(i-1, j));
        if(image->at<uchar>(i+1, j) == 0) neighbors.push_back(Vec2i(i+1, j));
        if(image->at<uchar>(i+1, j-1) == 0) neighbors.push_back(Vec2i(i+1, j-1));
        if(image->at<uchar>(i, j-1) == 0) neighbors.push_back(Vec2i(i, j-1));
        if(image->at<uchar>(i-1, j-1) == 0) neighbors.push_back(Vec2i(i-1, j-1));
    }

    // lower border
    else if (i == (rows - 1))
    {
        if(image->at<uchar>(i, j-1) == 0) neighbors.push_back(Vec2i(i, j-1));
        if(image->at<uchar>(i-1, j-1) == 0) neighbors.push_back(Vec2i(i-1, j-1));
        if(image->at<uchar>(i-1, j) == 0) neighbors.push_back(Vec2i(i-1, j));
        if(image->at<uchar>(i-1, j+1) == 0) neighbors.push_back(Vec2i(i-1, j+1));
        if(image->at<uchar>(i, j+1) == 0) neighbors.push_back(Vec2i(i, j+1));
    }

    // Normal case - pixel is somewhere
    // in the image, but not near any border.
    else
    {
        if(image->at<uchar>(i-1, j) == 0) neighbors.push_back(Vec2i(i-1, j));
        if(image->at<uchar>(i-1, j+1) == 0) neighbors.push_back(Vec2i(i-1, j+1));
        if(image->at<uchar>(i, j+1) == 0) neighbors.push_back(Vec2i(i, j+1));
        if(image->at<uchar>(i+1, j+1) == 0) neighbors.push_back(Vec2i(i+1, j+1));
        if(image->at<uchar>(i+1, j) == 0) neighbors.push_back(Vec2i(i+1, j));
        if(image->at<uchar>(i+1, j-1) == 0) neighbors.push_back(Vec2i(i+1, j-1));
        if(image->at<uchar>(i, j-1) == 0) neighbors.push_back(Vec2i(i, j-1));
        if(image->at<uchar>(i-1, j-1) == 0) neighbors.push_back(Vec2i(i-1, j-1));
    }

    return neighbors;
}

Vec2i pointToVec (Point p)
{
    return Vec2i(p.x, p.y);
}

vector<Vec2i> pointToVec (vector<Point> pl)
{
    vector<Vec2i> ret(pl.size());

    for(size_t i = 0; i < pl.size(); i++)
        ret[i] = Vec2i(pl[i].x, pl[i].y);

    return ret;
}

// Returns all black pixels that can be reached
// from the input pixel, including the pixel itself (DFS).
// Expects a binary (CV_U8 / CV_U8C1) matrix.
vector<Vec2i> getBlackComponentPixels (Vec2i pixel, Mat* image)
{   
    stack<Vec2i>* active = new stack<Vec2i>; // stack for new, unexpanded nodes
    vector<Vec2i>* found = new vector<Vec2i>; // list for expanded nodes
    vector<Vec2i>* connected = new vector<Vec2i>; // output list
    vector<Vec2i> currentBlackNeighbors; // contains all black neighbors of current
    Vec2i current; // pixel that is currently being evaluated

    if(image->type() != 0)
    {
        cout << "blackNeighbors: Matrix had " << image->channels() << " channels instead of 1!" << "\n";
        return *connected;
    }

    // if starting point is not black
    // return empty list
    if(image->at<uchar>(pixel[0], pixel[1]) != 0)
        return *connected;

    else
    {
        // add start pixel to open set and output
        // found->push_back(pixel);
        active->push(pixel);

        // search neighbors while
        // there are still unexpanded pixels
        while (!active->empty())
        {
            // Set the current pixel to the top pixel in the
            // stack and pop that pixel from the stack.
            current = active->top();
            active->pop();

            // if active pixel hasn't been found yet
            if(find(found->begin(), found->end(), current) == found->end())
            {
                // mark current as found
                found->push_back(current);

                // add current to output list
                connected->push_back(current);

                // retrieve all neighbors for the current pixel
                currentBlackNeighbors = eightConnectedBlackNeighbors(current, image);

                // add all black neighbors to the active stack
                for(auto neighbor = currentBlackNeighbors.begin(); neighbor != currentBlackNeighbors.end(); neighbor++)
                    if(find(found->begin(), found->end(), *neighbor) == found->end())
                        active->push(*neighbor);
            }
        }


        vector<Vec2i> ret = *connected;

        // cleanup
        //delete connected;
        //delete active;
        //delete found;

        return ret;
    }
}

// Erase all black pixels that belong to a component.
void eraseComponentPixels (ConnectedComponent comp, Mat* image)
{
    // Set component's seed as starting pixel
    Vec2i seed = comp.seed;

    eraseConnectedPixels(seed, image);
}

// Erase all black pixels connected to the input pixel.
void eraseConnectedPixels(Vec2i seed, Mat* image)
{
    // check if seed is out of bounds
    if(seed[0] < 0 || seed[0] > image->rows ||
            seed[1] < 0 || seed[1] > image->cols)
    {
        cout << "eraseComponentPixels: Seed of bounds!\n";
        cout << "Image was " << image->rows << ", " << image->cols << " (rows, cols)\n";
        cout << "Seed was " << seed[0] << ", " << seed[1] << "\n";

        return;
    }

    // get all black pixels that are reachable from the seed
    vector<Vec2i> pixels = getBlackComponentPixels(seed, image);

    // swap all black component pixels for white ones
    for(vector<Vec2i>::iterator pixel = pixels.begin(); pixel != pixels.end(); pixel++)
        (*image).at<uchar>((*pixel)[0], (*pixel)[1]) = 255;
}

/**
 * @brief Extract a black layer from an image. If a pixel
 * is below the supplied tresholds for each channel, the
 * pixel is assumed to be black.
 * @param input The input image in matrix form.
 * @param output The black layer in matrix form.
 */
void getBlackLayer(Vec3b thresholds, Mat input, Mat* output)
{
    for(int i = 0; i < input.rows; i++)
        for(int j = 0; j < input.cols; j++)
        {
            Vec3b currentPixel = input.at<Vec3b>(i, j);

            // check thresholds (Vec3b is BGR!)
            if(currentPixel[0] <= thresholds[0] &&
                    currentPixel[1] <= thresholds[1] &&
                    currentPixel[2] <= thresholds[2])
            {
                // pixel below all thresholds: make it black
                output->at<uchar>(i, j) = 0;
            }

            // else: make pixel white
            else
                output->at<uchar>(i, j) = 255;
        }

    namedWindow("black layer", CV_WINDOW_NORMAL);
    imshow("black layer", *output);
    //imwrite("BLACK.png", *output );
}


// checks if a MBR coordinate is invalid
bool isValidCoord (Vec2i* check)
{
    return (!(check[0] == Vec2i(INT_MAX, INT_MAX) || check[1] == Vec2i(-1, -1)));
}


// Calculates the area the MBR of a connected component occupies.
double getMBRArea(ConnectedComponent comp)
{
    // calculate component MBR area size
    Vec2i pmin = comp.mbr_min;
    Vec2i pmax = comp.mbr_max;

    double area;

    // component consists of only one pixel
    if(pmin == pmax)
        area = 1;

    else
    {
        double len_A = sqrt(pow(pmin[1] - pmax[1], 2) + pow(pmin[0] - pmin[0], 2));
        double len_B = sqrt(pow(pmin[1] - pmin[1], 2) + pow(pmin[0] - pmax[0], 2));

        // Account for cases in which the MBR is a 1px line
        if(len_A == 0)
            area = len_B;

        else if (len_B == 0)
            area = len_A;

        // otherwise: calculate MBR area normally
        else
            area = len_A * len_B;
    }

    return area;
}

// draw a line given in polar coordinates on the input image using the given color.
void drawLines (std::vector<Vec3f> lines, cv::Mat* image, Scalar color)
{
    for(size_t i = 0; i < lines.size(); i++)
    {
        // Convert polar line pair (theta, rho)
        // to cartesian coordinates (x, y) - the point
        // where the perpendicular line intersects
        // the origin line
        float poX = lines[i][1] * cos(lines[i][0]);
        float poY = lines[i][1] * sin(lines[i][0]);

        // Find cartesian equation equal to (theta, rho):
        // Find the slope of the line through the origin (0,0) and the
        // converted intersection point (rho, theta).
        // This line is perpendicular, so its slope is the inverse of
        // the origin line: m_new = (-1/m).
        // Check if xi or yi are 0, since if so the slope will be infinite (NaN)!
        double m = -1. / slope(0, 0, poX, poY);
        double b = -(m * poX) + poY;

        // Find two points on the line.
        double lp1_x = poX - image->cols;
        double lp1_y = m * lp1_x + b;
        Point pt1 = Point(lp1_x, lp1_y);

        double lp2_x = poX + image->cols;
        double lp2_y = m * lp2_x + b;
        Point pt2 = Point(lp2_x, lp2_y);

        //cout << "POLAR: " << "(" << lines[i][0] << ", " << lines[i][1]/0.0174533 << "), CART: " << pt1 << " to " << pt2 << "\n";

        line(*image, pt1, pt2, color, 1);
    }
}

// Calculates the cartesian distance between two points in 2D space.
double distanceBetweenPoints (Vec2f a, Vec2f b)
{
    int x1 = a[1];
    int y1 = a[0];

    int x2 = b[1];
    int y2 = b[0];

    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

// Calculates the distance between a point and a line.
double distanceFromCartesianLine(Vec2f point, pair<Vec2f, Vec2f> linePoints)
{
    double x = point[1];
    double y = point[0];

    double lp1_x = linePoints.first[1];
    double lp1_y = linePoints.first[0];

    double lp2_x = linePoints.second[1];
    double lp2_y = linePoints.second[0];

    double segLength = distanceBetweenPoints(linePoints.first, linePoints.second);

    if (segLength == 0.)
        return distanceBetweenPoints(point, linePoints.first);

    //double k = abs((lp2_y - lp1_y) * (x - lp1_x) - (lp2_x - lp1_x) * (y - lp1_y)) / (pow((lp2_y - lp1_y), 2) + pow((lp2_x - lp1_x), 2));
    double dist = abs((lp2_x - lp1_x) * (lp1_y - y) - (lp1_x - x) * (lp2_y - lp1_y)) / sqrt(pow((lp2_x - lp1_x), 2) + pow((lp2_y - lp1_y), 2));

    // perpendicular line intersection point on the original line
    // error here (still?)
    //Vec2f onLine = Vec2f(y + k * (lp2_x - lp1_x), x - k * (lp2_y - lp1_y));

    return dist;
}

// Calculates the distance between a point and a line segment.
// (= the length of the segment that is perpendicular to the
// line segment and ends at the supplied point)
double distanceFromCartesianSegment(Vec2f point, pair<Vec2f, Vec2f> linePoints)
{
    double x = point[1];
    double y = point[0];

    double lp1_x = linePoints.first[1];
    double lp1_y = linePoints.first[0];

    double lp2_x = linePoints.second[1];
    double lp2_y = linePoints.second[0];


    // find segment length
    double segLength = distanceBetweenPoints(linePoints.first, linePoints.second);

    // degenerated segment, so normal point to point distance is sufficient
    if (segLength == 0.)
        return distanceBetweenPoints(point, linePoints.first);

    // find squared segment length
    double sqLen = segLength * segLength;

    // Find out if the orthogonal tangent that passes through (x, y) exists
    double t = ((x - lp1_x) * (lp2_x - lp1_x) + (y - lp1_y) * (lp2_y - lp1_y)) / sqLen;


    // An orthogonal line from the segment that goes through (x, y) does not exist,
    // i.e. the segment would have to be extended in order for it to exist. Hence,
    // the shortest distance is the distance from (x, y) to the appropriate endpoint of
    // the line segment.

    // Use starting point
    if (t < 0.)
        return distanceBetweenPoints(point, linePoints.first);

    // Use ending point
    else if (t > 1.)
        return distanceBetweenPoints(point, linePoints.second);

    // Else, calculate the distance between (x, y) and the intersection
    // point on the line segment, which is the shortest distance.
    else
        return distanceBetweenPoints(point, Vec2f(lp1_y + t * (lp2_y - lp1_y), lp1_x + t * (lp2_x - lp1_x)));
}

// The signum function. Returns the sign of the input.
template <typename T> int sgn(T val)
{
    return (T(0) < val) - (val < T(0));
}

// Find out on which side of the line a point lies.
// Returns the output of the sign of the determinant:
//
// -1 = left of line, 0 = on the line, 1 = right of line
// (TODO: Check if left/right is correct)
int pointDirectionFromPolarLine (Vec2f point, Vec3f polarLine)
{
    double poX = (double) polarLine[1] * cos(polarLine[0]);
    double poY = (double) polarLine[1] * sin(polarLine[0]);

    double m = -1. / (slope(0,0, poX, poY));
    double b = -(m * poX) + poY;

    // Find two points on the line.
    double lp1_x = poX;
    double lp1_y =  m * lp1_x + b;

    double lp2_x = poX + 1;
    double lp2_y = m * lp2_x + b;

    // Point to be evaluated:
    double x = point[1];
    double y = point[0];

    // Calculate side:
    return sgn((lp2_x - lp1_x) * (y - lp1_y) - (lp2_y - lp1_y) * (x - lp1_x));

}


// Determines whether a point is enclosed by
// two PARALLEL polar lines.
//
// Returns true if it is, false if the lines aren't parallel or
// if the point is not between them.
bool pointBetweenPolarLines (Vec2f point, Vec3f polar1, Vec3f polar2)
{
    // check line parallelity
    /*if(!areParallelPolar(polar1, polar2))
        return false;*/

    int dir1 = pointDirectionFromPolarLine(point, polar1);
    int dir2 = pointDirectionFromPolarLine(point, polar2);

    // If the point lies on different sides
    // of two parallel lines, it can only lie
    // in between them! Otherwise, it is left or right
    // of both, and hence not in between.
    // The case in which a point is on either or both
    // of the lines is counted as "between" the lines.
    if((dir1 != dir2) || (dir1 == 0) || (dir2 == 0))
        return true;
    else
        return false;
}

// Calculates the slope of the line that goes through
// points (poX0, poY0) and (poX1, poY1)
double slope (double poX0, double poY0, double poX1, double poY1)
{
    double m;

    // vertical line
    if (poX1 == 0)
        m = INT_MAX;

    // horizontal line
    else if (poY1 == 0)
        m = 0;

    // normal slope
    else
        m = ((poY1 - poY0) / (poX1 - poX0));

    return m;
}

// Returns true if the two lines are parallel,
// false otherwise.
bool areParallelPolar (Vec3f polar1, Vec3f polar2)
{
    double poX1 = (double) polar1[1] * cos(polar1[0]);
    double poY1 = (double) polar1[1] * sin(polar1[0]);
    double poX2 = (double) polar2[1] * cos(polar2[0]);
    double poY2 = (double) polar2[1] * sin(polar2[0]);

    double m1 = -1. / slope(0, 0, poX1, poY1);
    double m2 = -1. / slope(0, 0, poX2, poY2);

    // two lines are parallel if they have
    // the same slope m:
    return (m1 == m2);
}

// Calculates the distance of a cartesian point to a polar line
// (for instance, the distance to Hough lines).
double distanceFromPolarLine (Vec2f point, Vec3f polarLine)
{
    // Convert polar line pair (theta, rho)
    // to cartesian coordinates (x, y) - the point
    // where the perpendicular line intersects
    // the origin line
    double poX = (double) polarLine[1] * cos(polarLine[0]);
    double poY = (double) polarLine[1] * sin(polarLine[0]);

    // Find cartesian equation equal to (theta, rho)
    //
    // TODO: Distance calculation should also be possible without
    // this transformation, but no idea how
    double m = -1. / (slope(0, 0, poX, poY));
    double b = -(m * poX) + poY;

    // Find two points on the line.
    double lp1_x = poX;
    double lp1_y =  m * lp1_x + b;

    double lp2_x = poX + 1;
    double lp2_y = m * lp2_x + b;

    // Calculate distance from input point to line
    // ( = length of the orthogonal tangent of the line that
    // passes through the input point).
    return distanceFromCartesianLine(point, make_pair(Vec2f(lp1_y, lp1_x), Vec2f(lp2_y, lp2_x)));
}

// Checks if a point is on a polar line or not.
// Uses a tolerance to cope with floating point values.
bool pointOnPolarLine (Vec2f point, Vec3f polarLine, double tolerance)
{
    double dist = distanceFromPolarLine(point, polarLine);

    if (dist <= tolerance)
        return true;
    else
        return false;
}


// Create a cluster around the primary cell with constant delta
// and varying rho.
// rhoStep denotes the rho resolution of the Hough domain, while
// numRho is the total number of rho values in the matrix.
void clusterCells (int totalNumberCells, float rhoStep, int numRho, Vec3f primaryCell, std::vector<Vec3f> *lines)
{
    if(totalNumberCells == 0)
        return;

    // cluster half the cells below and the other half above the primary cell
    int numberCells = totalNumberCells / 2; // Problem due to rounding uneven values?
    float theta = primaryCell[0];
    float rho = primaryCell[1];
    int base_addr = primaryCell[2];


    // Cluster totalNumberCells cells (constant theta) above and below the primary cell together with the cell itself
    // This results in a set of parallel lines in the cartesian domain, between or on which component centroids
    // may or may not lie.
    // This is done to catch small positional outliers, e.g. capital letter components whose centroids might not be
    // in line with the small letters in the same word.
    int rhoclst_start = rho - (numberCells * rhoStep) < 0 ? 0 : rho - (numberCells * rhoStep);
    int rhoclst_end = rho + (numberCells * rhoStep) > numRho * rhoStep ? numRho * rhoStep : rho + (numberCells * rhoStep);

    lines->clear();

    lines->push_back(Vec3f(theta, rhoclst_start, -1));
    lines->push_back(Vec3f(theta, rhoclst_end, -1));
}

// Finds the highest local difference in area size in
// a 5-neighborhood (if possible).
int localAreaDiff (vector<ConnectedComponent> cluster, int listPos, bool rev)
{
    int startPos, endPos;
    int maxdiff = -1;
    int n;

    cluster.size() >= 5 ? n = 2 : n = 1; // find feasible neighborhood size

    // Set neighbor positions to inspect
    if(!rev) // normal, to the right of input
    {
        startPos = listPos;
        listPos + n >= (int) cluster.size() ? endPos = (int) cluster.size() - 1 : endPos = listPos + n;
    }

    else // reverse, to the left of input
    {
        listPos - n < 0 ? startPos = 0 : startPos = listPos - n;
        endPos = listPos;
    }

    // calculate maximum area difference
    for(int i = startPos; i < endPos; i++)
        maxdiff = max(maxdiff, cluster.at(i+1).area - cluster.at(i).area);


    return maxdiff;
}
