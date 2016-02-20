#pragma once

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
    // Constructors:
    ConnectedComponent();
    ConnectedComponent(cv::Vec2i newmin, cv::Vec2i newmax, int newPixels, cv::Vec2i seed);
    ~ConnectedComponent();

    bool operator == (const ConnectedComponent other);

    // overwrite << operator to output information about this component
    friend std::ostream& operator << (std::ostream& stream, const ConnectedComponent& obj);

    cv::Vec2i mbr_min, mbr_max; // min and max coordinates of MBR
    cv::Vec2i seed; // (col, row)-Coordinates of the first found pixel of this component.
    cv::Vec2i centroid; // The midpoint of this component's MBR
    cv::Vec3f houghLine; // Hough line associated with this component (if any).
    int area; // total area occupied by this component's MBR
    int numBlackPixels; // number of black pixels in this component
};
