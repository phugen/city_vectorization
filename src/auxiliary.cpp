/**
  Contains various helper functions.
 */

#include "include/auxiliary.hpp"
#include "include/colorconversions.hpp"

using namespace std;
using namespace cv;

// checks if a pixel is purely black
/*bool isBlack (Vec3b check)
{
    return((check[0] == 0) && (check[1] == 0) && (check[2] == 0));
}*/

// checks if a pixel is purely black (refactor with type check?)
bool isBlack (uchar check)
{
    return (check == 0);
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
