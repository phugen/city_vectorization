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

// Maps Hough accumulator values to the pixel constraints of an image.
void mapHoughToImage (int rows, int cols, float theta, float rho, int numAngle, int numRho, int* accumulator)
{
    Mat overlay = Mat(rows, cols, CV_8U);
    overlay.setTo(0);

    int minacc = INT_MAX;
    int maxacc = 0;

    // re-usable cosine and sine values.
    float cos_v[numAngle];
    float sin_v[numAngle];

    for (int i = 0; i < numAngle; i++)
    {
        cos_v[i] = cos(i * theta);
        sin_v[i] = sin(i * theta);
    }

    // find min/max accum value
    for( int i = 0; i < rows; i++ )
        for( int j = 0; j < cols; j++ )
            for( int n = 0; n < numAngle; n++ )
            {
                int r = cvRound( j * cos_v[n] + i * sin_v[n] );
                r += (numRho - 1) / 2;

                int value = accumulator[(n+1) * (numRho+2) + r+1];
                minacc = min(value, minacc);
                maxacc = max(value, maxacc);
            }

    // normalize to 0 - 255 based on min and max values
    for( int i = 0; i < rows; i++ )
        for( int j = 0; j < cols; j++ )
            for(int n = 0; n < numAngle; n++ )
            {
                int r = cvRound( j * cos(n * theta) + i * sin(n * theta) );
                r += (numRho - 1) / 2;

                int value = accumulator[(n+1) * (numRho+2) + r+1];
                overlay.at<uchar>(i, j) = (255 * (value - minacc)) / (maxacc - minacc);
            }

    namedWindow("OVERLAY", WINDOW_AUTOSIZE);
    imshow("OVERLAY", overlay);
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
    // starting pixel for connecting pixel search
    Vec2i seed = comp.seed;

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

    namedWindow("black layer", WINDOW_AUTOSIZE);
    imshow("black layer", *output);
    imwrite("BLACK.png", *output );
}


// checks if a MBR coordinate is invalid
bool isValidCoord (Vec2i* check)
{
    return (!(check[0] == Vec2i(INT_MAX, INT_MAX) || check[1] == Vec2i(-1, -1)));
}


// draw a line given in polar coordinates on the input image using the given color.
void drawLines (std::vector<Vec2f> lines, cv::Mat* image, Scalar color)
{
    for(size_t i = 0; i < lines.size(); i++)
    {
        float rho = lines[i][0];
        float theta = lines[i][1];

        // To find the actual line, find the line that is perpendicular
        // to the line through the origin (0, 0) and (rho, theta) and
        // which intersects the line at (rho, theta).

        // convert intersection point coordinates from
        // polar to cartesian coordinates
        double xi = rho * cos(theta);
        double yi = rho * sin(theta);

        // Find the slope of the line through the origin (0,0) and the
        // converted intersection point (rho, theta).
        // This line is perpendicular, so its slope is the inverse of
        // the origin line: m_new = (-1/m).
        // Check if xi or yi are 0, since if so the slope will be infinite (NaN)!
        double m;

        // vertical line
        if (xi == 0)
            m = INT_MAX;

        // horizontal line
        else if (yi == 0)
            m = 0;

        // normal slope
        else
            m = -1. / ((yi - 0) / (xi - 0));

        // Since the intersection point (x, y) is known, we can now
        // solve the line equation y = m*x + b for b: b = -(m*x) + y
        double b = -(m * xi) + yi;

        // draw line across the whole image
        Point pt1(0, m * 0 + b);
        Point pt2(image->cols, m * (image->cols) + b);

        // choose line color automatically based on range (0, size_of_list)
        //Scalar color = intToRGB(Vec2i(0, lines.size()), i);

        line(*image, pt1, pt2, color, 1);
    }
}

// Calculates the cartesian distance between two points in 2D space.
double distanceBetweenPoints (Vec2i a, Vec2i b)
{
    int x1 = a[1];
    int y1 = a[0];

    int x2 = b[1];
    int y2 = b[0];

    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

// Calculates the distance between a point and a line segment.
// (= the length of the segment that is perpendicular to the
// line segment and ends at the supplied point)
double distanceFromCartesianLine(Vec2i point, pair<Vec2i, Vec2i> linePoints)
{
    int x = point[1];
    int y = point[0];

    int lp1_x = linePoints.first[1];
    int lp1_y = linePoints.first[0];

    int lp2_x = linePoints.second[1];
    int lp2_y = linePoints.second[0];


    // find length of the line segment
    double segLength = distanceBetweenPoints(linePoints.first, linePoints.second);

    // degenerated segment, so normal point to point distance is sufficient
    if (segLength == 0.)
        return distanceBetweenPoints(point, linePoints.first);

    double t = ((x - lp1_x) * (lp2_x - lp1_x) + (y - lp1_y) * (lp2_y - lp1_y)) / segLength;

    // point is on the infinite line, so find out which
    // line endpoint to use for the segment distance
    if (t < 0.)
        return distanceBetweenPoints(point, linePoints.first);
    if (t > 1.)
        return distanceBetweenPoints(point, linePoints.second);


    // else, use normal distance to point
    return distanceBetweenPoints(point, Vec2i(lp1_y + t * (lp2_y - lp1_y), lp1_x + t * (lp2_x - lp1_x)));
}


// Calculates the distance of a cartesian point to a polar line
// (for instance, the distance to Hough lines).
double distanceFromPolarLine (Vec2f point, Vec2f polarLine)
{
    // cartesian coordinates of the point
    double x = point[1];
    double y = point[0];

    // convert polar line pair (rho, theta)
    // to cartesian coordinates (x, y) - the point
    // where the perpendicular line intersects
    // the origin line
    float poX = polarLine[0] * cos(polarLine[1]);
    float poY = polarLine[0] * sin(polarLine[1]);

    // find cartesian equation equal to (rho, theta)
    // (should also be possible directly but no idea how)
    double m; poX == 0 ? m = (-1. / INT_MAX) : m = -1. / ((poY - 0) / (poX - 0));
    double b = -(m * poX) + poY;

    // find two points on the line.
    Point begin = Point(0, (m * 0 + b));
    Point end = Point (100000, (m * 100000 + b)); // unsafe - should be image->cols instead

    // Calculate input point distance from line.
    double normalLength = hypot(end.x - begin.x, end.y - begin.y);
    double dist = (double) abs(((x - begin.x) * (end.y - begin.y) - (y - begin.y) * (end.x - begin.x)) / normalLength);

    return dist;
}

// Checks if a point is on a polar line or not.
// Uses a tolerance to cope with floating point values.
bool pointOnPolarLine (Vec2f point, Vec2f polarLine, double tolerance)
{
    if (distanceFromPolarLine(point, polarLine) <= tolerance)
        return true;
    else
        return false;
}


// Create numberRho new lines centered around the current Hough rho cell,
// while keeping the angle theta constant.
// rhoStep denotes the rho resolution of the Hough domain, while
// numRho is the total number of rho values in the matrix.
void clusterCells (int totalNumberCells, float rhoStep, int numRho, Vec2f primaryCell, vector<Vec2f>* clusterLines)
{
    if(totalNumberCells == 0)
        return;

    // generate half the lines below and the other half above the primary cell
    int numberCells = totalNumberCells / 2; // Problem due to rounding uneven values?
    float lRho = primaryCell[0];
    float lTheta = primaryCell[1];

    // start and end (numberCells * rho step) above and below current cell
    // if possible, otherwise cap at bounds
    int rhoclst_start = lRho - (numberCells * rhoStep) < 0 ? 0 : lRho - (numberCells * rhoStep);
    int rhoclst_end = lRho + (numberCells * rhoStep) > numRho * rhoStep ? numRho * rhoStep : lRho + (numberCells * rhoStep);

    // For each accumulator cell (= each accepted line) above the current threshold:
    // Cluster the five rho cells (constant theta) above and below it together with the cell itself
    // This results in a set of a maximum of 11 parallel lines in the cartesian domain, on which component centroids
    // may or may not lie.
    // This is done to catch outliers, i.e. capital letter components whose centroids might not be
    // in line with the small letters in the same word by improving the guess for the rho resolution.
    clusterLines->clear();

    for(double z = rhoclst_start; z <= rhoclst_end; z+=rhoStep)
        clusterLines->push_back(Vec2f(z, lTheta));
}
