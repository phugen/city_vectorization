/**
  * This class describes a connected component in a binary black/white image,
  * e.g. a mass of black pixels in which each pixel is connected
  * to all other pixels of the component by (indirect) eight-connection.
  *
  * Author: Phugen
  */

#include "include/connectedcomponent.hpp"
#include "include/auxiliary.hpp"

using namespace std;
using namespace cv;

ConnectedComponent::ConnectedComponent()
{}

ConnectedComponent::ConnectedComponent(Vec2i newmin, Vec2i newmax, int newPixels, Vec2i seed)
{
    this->mbr_min = newmin;
    this->mbr_max = newmax;

    int x = (newmin[0] + newmax[0]) / 2;
    int y = (newmin[1] + newmax[1]) / 2;
    this->centroid = Vec2i(x, y);

    //houghLine = NULL;

    this->seed = seed;
    this->numBlackPixels = newPixels;
}

ConnectedComponent::~ConnectedComponent(){}

bool ConnectedComponent::operator == (const ConnectedComponent other)
{
    // define equality as having the same bounding box
    // (which isn't really exact but suffices for our needs)
    if(mbr_min == other.mbr_min && mbr_max == other.mbr_max)
        return true;
    else
        return false;
}

ostream& operator << (ostream& stream, const ConnectedComponent& obj)
{
    int pxInMBR = (((obj.mbr_max[1] + 1) - obj.mbr_min[1]) * ((obj.mbr_max[0] + 1) - obj.mbr_min[0]));

    return stream << "[" << obj.mbr_min << " to " << obj.mbr_max << "] = " << pxInMBR << " pixels, " << obj.numBlackPixels << " of those black (" << (((float) obj.numBlackPixels / (float) pxInMBR) * 100) << "%).\n";
}


