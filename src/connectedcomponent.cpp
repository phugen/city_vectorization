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

ostream& operator<<(ostream& stream, const ConnectedComponent& obj)
{
    int pxInMBR = (((obj.mbr_max[0] + 1) - obj.mbr_min[0]) * ((obj.mbr_max[1] + 1) - obj.mbr_min[1]));

    return stream << "[" << obj.mbr_min << " to " << obj.mbr_max << "] = " << pxInMBR << " pixels, " << obj.numPixels << " of those black (" << (((float) obj.numPixels / (float) pxInMBR) * 100) << "%).\n";
}


