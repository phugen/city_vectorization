/**
  * Contains functions dealing with converting color values
  * from one color space to another.
  *
  * Author: Philipp Hugenroth
  */

#include "include/colorconversions.hpp"
#include "include/opencvincludes.hpp"
#include <iostream>

using namespace std;
using namespace cv;


// r,g,b values are from 0 to 1
// h = [0,360], s = [0,1], v = [0,1]
//		if s == 0, then h = -1 (undefined)
Vec3i RGBtoHSV (Vec3i rgb)
{
    float h, s, v;
    float minVal, maxVal, delta;

    float r = (float) rgb[0];
    float g = (float) rgb[1];
    float b = (float) rgb[2];


    minVal = min(r, min(g, b));
    maxVal = min(r, min(g, b));
    v = maxVal;

    delta = maxVal - minVal;

    if(maxVal != 0)
        s = delta / maxVal;
    else
    {
        // r = g = b = 0
        // s = 0, v is undefined
        s = 0;
        h = -1;

        return Vec3i(0, 0, 0);
    }

    if(r == maxVal)
        h = (g - b) / delta;		// between yellow & magenta

    else if(g == maxVal)
        h = 2 + (b - r) / delta;	// between cyan & yellow

    else
        h = 4 + (r - g) / delta;	// between magenta & cyan

    h *= 60;				// degrees
    if(h < 0)
        h += 360;


    return Vec3i((int) h, (int) s, (int) v);
}


Vec3i HSVtoRGB(Vec3i hsv)
{
    int i;
    float f, p, q, t;
    float r, g, b;

    float h = (float) hsv[0];
    float s = (float) hsv[1];
    float v = (float) hsv[2];



    if(s == 0)
    {
        // achromatic (grey)
        r = g = b = v;

        return Vec3i(255 * b, 255 * g, 255 * r);
    }

    h /= 60;			// sector 0 to 5
    i = floor(h);
    f = h - i;			// factorial part of h
    p = v * ( 1 - s );
    q = v * ( 1 - s * f );
    t = v * ( 1 - s * ( 1 - f ) );

    switch( i )
    {
    case 0:
        r = v;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = v;
        b = p;
        break;
    case 2:
        r = p;
        g = v;
        b = t;
        break;
    case 3:
        r = p;
        g = q;
        b = v;
        break;
    case 4:
        r = t;
        g = p;
        b = v;
        break;
    default:		// case 5:
        r = v;
        g = p;
        b = q;
        break;
    }

    //cout << ", RGB: (" << r << "," << g << "," << b << "\n\n";
    return Vec3i(255 * b, 255 * g, 255 * r);
}

// maps an integer range to RGB color vectors.
Vec3i intToRGB (Vec2i range, int value)
{
    //float min = (float) range[0];
    float max = (float) range[1];

    int h = (360 / max) * value;

    Vec3i hsv = Vec3i(h, 1, 1);

    return HSVtoRGB(hsv);
}
