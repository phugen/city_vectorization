#include "include/connectedcomponent.hpp"

using namespace std;
using namespace cv;

ConnectedComponent::ConnectedComponent(Vec2i newmin, Vec2i newmax, int newPixels, Vec2i seed)
{
    mbr_min = newmin;
    mbr_max = newmax;

    int x = (newmin[0] + newmax[0]) / 2;
    int y = (newmin[1] + newmax[1]) / 2;
    centroid = Vec2i(x, y);

    this->seed = seed;
    numPixels = newPixels;
}

ConnectedComponent::~ConnectedComponent()
{}


// Returns 1 if the lines intersect, otherwise 0. In addition, if the lines
// intersect the intersection point may be stored in the floats i_x and i_y.
/*bool get_line_intersection(float p0_x, float p0_y, float p1_x, float p1_y,
    float p2_x, float p2_y, float p3_x, float p3_y)
{
    float s1_x, s1_y, s2_x, s2_y;
    s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
    s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

    float s, t;
    s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
    t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
        return true;
    }

    return false; // No collision
}


bool ConnectedComponent::intersected(float rho, float theta)
{
    // calculate point from rho and theta values
    double a = cos(theta), b = sin(theta);
    double x0 = a * rho, y0 = b * rho;

    // Hessian normal form to slope-intercept form:
    //     r = x * cos(theta) + y * sin(theta)
    // (=) y = x * ((-cos(theta)/sin(theta)) + (r / sin(theta))
    //
    // handle y = 0 as in http://stackoverflow.com/questions/28351804/hough-transform-converted-polar-coordinates-back-to-cartesian-but-still-cant

}*/


ostream& operator<<(ostream& stream, const ConnectedComponent& obj)
{
    int pxInMBR = (((obj.mbr_max[0] + 1) - obj.mbr_min[0]) * ((obj.mbr_max[1] + 1) - obj.mbr_min[1]));

    return stream << "[" << obj.mbr_min << " to " << obj.mbr_max << "] = " << pxInMBR << " pixels, " << obj.numPixels << " of those black (" << (((float) obj.numPixels / (float) pxInMBR) * 100) << "%).\n";
}


