#include "medoidHough.hpp"
#include "opencvincludes.hpp"
#include "connectedcomponent.hpp"

#include <vector>

using namespace std;
using namespace cv;

// euclidial distance for MBR centroids.
float centroidDistance (Vec2i a, Vec2i b)
{
    return sqrt(pow((a[0] - b[0]), 2) + pow((a[1] - b[1]), 2));
}


// comparator function
bool smallerDistance (Vec3i a, Vec3i b)
{
    return (a[2] < b[2]);
}


// an altered version of the k-Medoids algorithm, intended to find
// the eleven closest MBRs
void medoidHough (ConnectedComponent center, vector<ConnectedComponent> cluster)
{

}
