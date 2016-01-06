#ifndef CONNECTEDCOMPONENT_HPP
#define CONNECTEDCOMPONENT_HPP

#include "opencvincludes.hpp"

/**
 * @brief A class containing information about
 * a connected component found in an image, as detailed
 * in
 *
 * L. A. Fletcher and R. Kasturi,
 * “A robust algorithm for text string separation from mixed text and graphics images"
 *
 * (Section: "Connected Component Generation")
 */
class ConnectedComponent
{
    public:
        // left those vars public because the way they're retrieved
        // is not likely to be changed, so no need to encapsulate them

        cv::Vec2i mbr_min, mbr_max; // min and max coordinates of MBR
        cv::Vec2i bot, top; // coordinates of top and bottom seeds (... find out what that is!)
        cv::Vec2i centroid; // The midpoint of this component's MBR
        int numPixels; // number of black pixels in this component

        // Constructors:
       ConnectedComponent(cv::Vec2i newmin, cv::Vec2i newmax, int newPixels);
       ~ConnectedComponent();

       // Methods:

       // Returns true if the MBR of this component is intersected by the
       // Hessian normal form line = (theta, rho), or if the line
       // is its tangent.
       //bool intersected (float rho, float theta);

       // overwrite << operator to output information about this component
       friend std::ostream& operator << (std::ostream& stream, const ConnectedComponent& obj);
};

#endif // CONNECTEDCOMPONENT_HPP
