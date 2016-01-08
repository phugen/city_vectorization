/**
  Contains various helper functions.
 */

#include "include/auxiliary.hpp"
#include "include/colorconversions.hpp"
#include "include/connectedcomponent.hpp"

#include <queue>
#include <list>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace cv;

// checks if a pixel is purely black
/*bool isBlack (Vec3b check)
{
    return((check[0] == 0) && (check[1] == 0) && (check[2] == 0));
}*/

// checks if a pixel is purely blac
bool isBlack (uchar check)
{
    return (check == 0);
}

// Returns a list of all eight-connected neighbors
// of the input pixel.
//
// DON'T mistake this for the check in unionfindcomponents.cpp, which
// operates in a scanline fashion plus checks for black pixels, and thus
// gives different neighbors for a pixel.
vector<Vec2i> eightConnectedNeighbors(Vec2i pixel, Mat* image)
{
    vector<Vec2i> neighbors; // output list
    int cols = image->cols;
    int rows = image->rows;
    int i = pixel[0];
    int j = pixel[1];

    // top left pixel
    if(i == 0 && j == 0)
    {
        neighbors.push_back(Vec2i(i+1, j));
        neighbors.push_back(Vec2i(i+1, j+1));
        neighbors.push_back(Vec2i(i, j+1));
    }

    // top right pixel
    else if(i == 0 && j == (cols - 1))
    {
        neighbors.push_back(Vec2i(i+1, j));
        neighbors.push_back(Vec2i(i+1, j-1));
        neighbors.push_back(Vec2i(i, j-1));
    }


    // bottom left pixel
    else if (i == (rows - 1) && j == 0)
    {
        neighbors.push_back(Vec2i(i, j+1));
        neighbors.push_back(Vec2i(i-1, j+1));
        neighbors.push_back(Vec2i(i-1, j));
    }

    // bottom right pixel
    else if (i == (rows - 1) && j == (cols - 1))
    {
        neighbors.push_back(Vec2i(i-1, j));
        neighbors.push_back(Vec2i(i-1, j-1));
        neighbors.push_back(Vec2i(i, j-1));
    }

    // upper border
    else if (i == 0)
    {
        neighbors.push_back(Vec2i(i, j+1));
        neighbors.push_back(Vec2i(i-1, j+1));
        neighbors.push_back(Vec2i(i-1, j));
        neighbors.push_back(Vec2i(i-1, j-1));
        neighbors.push_back(Vec2i(i, j-1));
    }

    // left border
    else if (j == 0)
    {
        neighbors.push_back(Vec2i(i-1, j));
        neighbors.push_back(Vec2i(i-1, j+1));
        neighbors.push_back(Vec2i(i, j+1));
        neighbors.push_back(Vec2i(i+1, j+1));
        neighbors.push_back(Vec2i(i+1, j));
    }

    // right border
    else if (j == (cols - 1))
    {
        neighbors.push_back(Vec2i(i-1, j));
        neighbors.push_back(Vec2i(i+1, j));
        neighbors.push_back(Vec2i(i+1, j-1));
        neighbors.push_back(Vec2i(i, j-1));
        neighbors.push_back(Vec2i(i-1, j-1));
    }

    // lower border
    else if (i == (rows - 1))
    {
        neighbors.push_back(Vec2i(i, j-1));
        neighbors.push_back(Vec2i(i-1, j-1));
        neighbors.push_back(Vec2i(i-1, j));
        neighbors.push_back(Vec2i(i-1, j+1));
        neighbors.push_back(Vec2i(i, j+1));
    }

    // Normal case - pixel is somewhere
    // in the image, but not near any border.
    else
    {
        neighbors.push_back(Vec2i(i-1, j));
        neighbors.push_back(Vec2i(i-1, j+1));
        neighbors.push_back(Vec2i(i, j+1));
        neighbors.push_back(Vec2i(i+1, j+1));
        neighbors.push_back(Vec2i(i+1, j));
        neighbors.push_back(Vec2i(i+1, j-1));
        neighbors.push_back(Vec2i(i, j-1));
        neighbors.push_back(Vec2i(i-1, j-1));
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

// Returns all corner pixels that are reachable from the input
// CORNER PIXEL(!). It is expected that the input pixel is not in the
// corner input vector.
//
// Once a corner is hit, the search is discontinued in
// that "direction".
//
// It is assumed that image is binary black/white,
// with white being the background color, and that
// the image was thinned so all possible paths
// are only one pixel wide.
//
// Since the code is very similiar, see getBlackComponentPixels for code commentary.
vector<Vec2i> getNearestCorners(vector<Vec2i> corners, Vec2i pixel, Mat* image, Mat* reconstructed)
{
    queue<Vec2i> active;
    vector<Vec2i> found;
    vector<Vec2i> corners_reachable;

    Mat test;
    cvtColor(*image, test, CV_GRAY2RGB);
    namedWindow("nearest", WINDOW_AUTOSIZE);


    if(!isBlack(image->at<uchar>(pixel[1], pixel[0])))
        return corners_reachable;

    else
    {
        active.push(pixel);
        found.push_back(pixel);


        while (!active.empty())
        {
            Vec2i current = active.front();
            vector<Vec2i> currentNeighbors = eightConnectedNeighbors(current, image);

            for(vector<Vec2i>::iterator neighbor = currentNeighbors.begin(); neighbor != currentNeighbors.end(); neighbor++)
            {
                if((find(found.begin(), found.end(), *neighbor) == found.end()) &&
                        isBlack(image->at<uchar>(((*neighbor)[1]), (*neighbor)[0])))
                {
                    // neighbor is a corner
                    if(find(corners.begin(), corners.end(), *neighbor) != corners.end())
                    {
                        corners_reachable.push_back(*neighbor);
                        found.push_back(*neighbor);

                        Point p = Point((*neighbor)[1], (*neighbor)[0]);
                        rectangle(test, p, p, Scalar(255, 0, 0), 1, 8, 0);
                        //imshow("nearest", test);

                        break; // stop search in this direction
                    }

                    // neighbor is a regular black pixel
                    else
                    {
                        active.push(*neighbor);
                        found.push_back(*neighbor);
                    }
                }
            }

            active.pop();
        }

        // debug lines
        for(auto iter = corners_reachable.begin(); iter != corners_reachable.end(); iter++)
        {
            line(*reconstructed, Point(pixel), Point(*iter), Scalar(0,0,0), 1);
            //putText(testimage, to_string(iterno), current, FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0,255,0), 1, 8, false);
        }

        cout << "#Corners reachable: " << corners_reachable.size() << "\n";
        return corners_reachable;
    }
}

// Returns all black pixels that can be reached
// from the input pixel, including the pixel itself.
// Expects a greyscale matrix.
vector<Vec2i> getBlackComponentPixels (Vec2i pixel, Mat* image)
{
    queue<Vec2i> active; // queue for new, unexpanded nodes
    vector<Vec2i> found; // list for expanded nodes
    vector<Vec2i> connected; // output list

    if(image->channels() > 1)
    {
        cout << "blackNeighbors: Matrix had " << image->channels() << " channels instead of 1!" << "\n";
        return connected;
    }

    // if starting point is not black
    // return empty list
    if(!isBlack(image->at<uchar>(pixel[1], pixel[0])))
        return connected;

    else
    {
        // add start pixel to open set and output
        active.push(pixel);
        connected.push_back(pixel);
        found.push_back(pixel);

        // search neighbors while
        // there are still unexpanded pixels
        while (!active.empty())
        {
            // take next unexpanded node
            Vec2i current = active.front();

            // retrieve all neighbors for the current pixel
            vector<Vec2i> currentNeighbors = eightConnectedNeighbors(current, image);

            // look at black neighbors
            // if they haven't been expanded yet (= that are not in the "found" list)
            for(vector<Vec2i>::iterator neighbor = currentNeighbors.begin(); neighbor != currentNeighbors.end(); neighbor++)
            {
                if((find(found.begin(), found.end(), *neighbor) == found.end()) &&
                        isBlack(image->at<uchar>(((*neighbor)[1]), (*neighbor)[0])))
                {
                    connected.push_back(*neighbor);
                    active.push(*neighbor);
                    found.push_back(*neighbor); // mark as found
                }
            }

            // erase current pixel from active set
            // and make the next pixel the current pixel
            active.pop();
        }

        cout << "#Black component pixels: " << connected.size() << "\n";
        return connected;
    }
}

// Erase all black pixels that belong to a component.
// TODO: Swap "seed" for Component object.
void eraseComponentPixels (ConnectedComponent comp, Mat* image)
{
    Vec2i seed = comp.seed;

    // check if seed is out of bounds
    if(seed[0] < 0 || seed[0] > image->cols ||
            seed[1] < 0 || seed[1] > image->rows)
    {
        cout << "eraseComponentPixels: Seed of bounds!\n";
        cout << "Image was " << image->rows << ", " << image->cols << " (rows, cols)\n";
        cout << "Seed was " << seed[1] << ", " << seed[0] << "\n";

        return;
    }

    // get all black pixels that are reachable from the seed
    vector<Vec2i> pixels = getBlackComponentPixels(seed, image);

    // swap all black component pixels for white ones
    for(vector<Vec2i>::iterator pixel = pixels.begin(); pixel != pixels.end(); pixel++)
        (*image).at<uchar>((*pixel)[1], (*pixel)[0]) = 255;

    cout << "DONE" << "\n";
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
    for(int i = 0; i < input.cols; i++)
    {
        for(int j = 0; j < input.rows; j++)
        {
            Vec3b currentPixel = input.at<Vec3b>(j, i);

            // check thresholds (Vec3b is BGR!)
            if(currentPixel[0] <= thresholds[0] &&
                    currentPixel[1] <= thresholds[1] &&
                    currentPixel[2] <= thresholds[2])
            {
                // pixel below all thresholds: make it black
                output->at<uchar>(j, i) = 0;
            }

            // else: make pixel white
            else
                output->at<uchar>(j, i) = 255;
        }
    }

    namedWindow("black layer", WINDOW_AUTOSIZE);
    imshow("black layer", *output);
}


// checks if a MBR coordinate is invalid
bool isValidCoord (Vec2i* check)
{
    //return (check != NULL);
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

        // Find the slope of the line through (0,0) and the
        // converted intersection point (rho, theta).
        // This line is perpendicular, so its slope is the inverse of
        // the origin line: m_new = (-1/m).
        double m = -1. / ((yi - 0) / (xi - 0));

        // Since the intersection point (x, y) is known, we can now
        // solve the line equation y = m*x + b for b: b = -(m*x) + y
        double b = -(m * xi) + yi;

        // draw line across the whole image
        Point pt1(0, m * 0 + b);
        Point pt2(image->cols, m * (image->cols) + b);

        // choose line color automatically based on range (0, size_of_list)
        Scalar color = intToRGB(Vec2i(0, lines.size()), i);

        line(*image, pt1, pt2, color, 1);
    }
}


// Checks if a point is on a polar line or not.
// Uses a tolerance to cope with float/double values.
bool pointOnPolarLine (Vec2f point, Vec2f polarLine, double tolerance)
{
    // cartesian coordinates of the point
    double x = point[0];
    double y = point[1];

    // convert polar line pair (rho, theta)
    // to cartesian coordinates (x, y) - the point
    // where the perpendicular line intersects
    // the origin line
    float poX = polarLine[0] * cos(polarLine[1]);
    float poY = polarLine[0] * sin(polarLine[1]);

    // find cartesian equation equal to (rho, theta)
    // (should also be possible directly but no idea how)
    double m = -1. / ((poY - 0) / (poX - 0));
    double b = -(m * poX) + poY;

    // plug in point and check if equation holds (within tolerance)
    // if yes, then the point is on the line.
    double res = m * x + b;

    if (abs(y - res) <= tolerance)
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
    for(int z = rhoclst_start; z <= rhoclst_end; z+=rhoStep)
        clusterLines->push_back(Vec2f(z, lTheta));
}
