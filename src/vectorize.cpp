#include "include/vectorize.hpp"
#include "include/cairo/cairo.h"
#include "include/cairo/cairo-svg.h"

#include <iostream>

using namespace std;
using namespace cv;

// Create a .svg file "filename.svg" based on the input raster image.
// Expects binary image (i.e. image passed blacklayer filter before).
void vectorizeImage (Mat* image, string filename)
{
    cout << "\n ------------------------- \n";
    cout << "Starting vectorization ...\n";

    // Invert image; canny not needed
    Mat inverted;
    bitwise_not(*image, inverted);

    // Find line segments
    vector<Vec4f> lines;
    HoughLinesP(inverted, lines, 1, 0.0174533, 15, 0, 0);

    cout << lines.size() << " line segments detected!\n";

    // debug: overlay found lines on image in blue
    cvtColor(*image, *image, CV_GRAY2RGB);
    Mat vectoronly = Mat(image->rows, image->cols, image->type());

    for(auto l = lines.begin(); l != lines.end(); l++)
    {
        Vec4f cur = *l;
        Point pt1 = Point(cur[0], cur[1]);
        Point pt2 = Point(cur[2], cur[3]);

        line(vectoronly, pt1, pt2, Scalar(255, 0, 0), 1);
    }

    cout << "Writing segment descriptions to " << filename << ".svg ...\n";

    // Set up cairo canvas
    cairo_surface_t *surface;
    cairo_t *cr;
    surface = cairo_svg_surface_create((filename + ".svg").c_str(), image->cols, image->rows);
    cr = cairo_create(surface);
    cairo_set_source_rgb(cr, 0, 0, 0); // set background color
    cairo_set_line_width(cr, 1); // set line width
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE); // don't smoothen line ends

    // Write line segment descriptions to .svg file
    for(auto l = lines.begin(); l != lines.end(); l++)
    {
        Vec4f cur = *l;
        cairo_move_to(cr, cur[0], cur[1]);
        cairo_line_to(cr, cur[2], cur[3]);
        cairo_stroke(cr);
    }

    // Free canvas
    cairo_surface_destroy(surface);
    cairo_destroy(cr);

    cout << "Vectorization done!";

    namedWindow("VECTORIZED", WINDOW_NORMAL);
    imshow("VECTORIZED", vectoronly);
}
