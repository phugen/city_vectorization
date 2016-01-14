
/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Copyright (C) 2013, OpenCV Foundation, all rights reserved.
// Copyright (C) 2014, Itseez, Inc, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "include/customhoughtransform.hpp"
#include <iostream>

using namespace std;
using namespace cv;


struct hough_cmp_gt
{
    hough_cmp_gt(const int* _aux) : aux(_aux) {}
    bool operator()(int l1, int l2) const
    {
        return aux[l1] > aux[l2] || (aux[l1] == aux[l2] && l1 < l2);
    }
    const int* aux;
};



/** Taken from the official openCV code and altered to expose access to the accumulator
* by adding it to the supplied "accumulator" array pointer.
*/

/*
Here image is an input raster;
step is it's step; size characterizes it's ROI;
rho and theta are discretization steps (in pixels and radians correspondingly).
threshold is the minimum number of pixels in the feature for it
to be a candidate for line. lines is the output
array of (rho, theta) pairs. linesMax is the buffer size (number of pairs).
Functions return the actual number of found lines.
*/
void HoughLinesCustom( const cv::Mat& img, float rho, float theta,
                       double min_theta, double max_theta, int* accumulator)
{
    int i, j;
    float irho = 1 / rho;

    CV_Assert( img.type() == CV_8UC1 );

    const uchar* image = img.ptr();
    int step = (int)img.step;
    int width = img.cols;
    int height = img.rows;

    if (max_theta < min_theta ) {
        CV_Error( CV_StsBadArg, "max_theta must be greater than min_theta" );
    }
    int numangle = cvRound((max_theta - min_theta) / theta);
    int numrho = cvRound(((width + height) * 2 + 1) / rho);

#if defined HAVE_IPP && !defined(HAVE_IPP_ICV_ONLY) && IPP_VERSION_X100 >= 810 && IPP_DISABLE_BLOCK
    CV_IPP_CHECK()
    {
        IppiSize srcSize = { width, height };
        IppPointPolar delta = { rho, theta };
        IppPointPolar dstRoi[2] = {{(Ipp32f) -(width + height), (Ipp32f) min_theta},{(Ipp32f) (width + height), (Ipp32f) max_theta}};
        int bufferSize;
        int nz = countNonZero(img);
        int ipp_linesMax = std::min(linesMax, nz*numangle/threshold);
        int linesCount = 0;
        lines.resize(ipp_linesMax);
        IppStatus ok = ippiHoughLineGetSize_8u_C1R(srcSize, delta, ipp_linesMax, &bufferSize);
        Ipp8u* buffer = ippsMalloc_8u(bufferSize);
        if (ok >= 0) ok = ippiHoughLine_Region_8u32f_C1R(image, step, srcSize, (IppPointPolar*) &lines[0], dstRoi, ipp_linesMax, &linesCount, delta, threshold, buffer);
        ippsFree(buffer);
        if (ok >= 0)
        {
            lines.resize(linesCount);
            CV_IMPL_ADD(CV_IMPL_IPP);
            return;
        }
        lines.clear();
        setIppErrorStatus();
    }
#endif

    AutoBuffer<int> _accum((numangle+2) * (numrho+2));
    std::vector<int> _sort_buf;
    AutoBuffer<float> _tabSin(numangle);
    AutoBuffer<float> _tabCos(numangle);
    int *accum = _accum;
    float *tabSin = _tabSin, *tabCos = _tabCos;

    memset( accum, 0, sizeof(accum[0]) * (numangle+2) * (numrho+2) );

    float ang = static_cast<float>(min_theta);
    for(int n = 0; n < numangle; ang += theta, n++ )
    {
        tabSin[n] = (float)(sin((double)ang) * irho);
        tabCos[n] = (float)(cos((double)ang) * irho);
    }

    // stage 1. fill accumulator
    for( i = 0; i < height; i++ )
        for( j = 0; j < width; j++ )
        {
            if( image[i * step + j] != 0 )
                for(int n = 0; n < numangle; n++ )
                {
                    int r = cvRound( j * tabCos[n] + i * tabSin[n] );
                    r += (numrho - 1) / 2;
                    accum[(n+1) * (numrho+2) + r+1]++;
                    accumulator[(n+1) * (numrho+2) + r+1]++; // add to exposed accum; zero matrix or other sane value init expected.
                }
        }

    // stage 2. find local maximums
    /*for(int r = 0; r < numrho; r++ )
        for(int n = 0; n < numangle; n++ )
        {
            int base = (n+1) * (numrho+2) + r+1;
            if( accum[base] > threshold &&
                    accum[base] > accum[base - 1] && accum[base] >= accum[base + 1] &&
                    accum[base] > accum[base - numrho - 2] && accum[base] >= accum[base + numrho + 2] )
                _sort_buf.push_back(base);
        }

    // stage 3. sort the detected lines by accumulator value
    std::sort(_sort_buf.begin(), _sort_buf.end(), hough_cmp_gt(accum));

    // stage 4. store the first min(total,linesMax) lines to the output buffer
    linesMax = std::min(linesMax, (int)_sort_buf.size());
    double scale = 1./(numrho+2);
    for( i = 0; i < linesMax; i++ )
    {
        LinePolar line;
        int idx = _sort_buf[i];
        int n = cvFloor(idx*scale) - 1;
        int r = idx - (n+1)*(numrho+2) - 1;
        line.rho = (r - (numrho - 1)*0.5f) * rho;
        line.angle = static_cast<float>(min_theta) + n * theta;
        lines.push_back(Vec2f(line.rho, line.angle));
    }*/


}

// Extracts polar Hough lines from a Hough accumulator.
//
// THRESH_GT is the normal thresholding mode,
// i.e. all values >= threshold are considered.
//
// THRESH_EQ only extracts those lines which stem from
// an accumulator value which is == threshold.
//
// Contributions is a vector that will contain the (numrho, numtheta)
// pairs that the line at the same vector position in "lines" contri-
// buted to the accumulator, so they can be deleted individually.
void HoughLinesExtract (int* accum, int numrho, int numangle, float rho, float theta, float min_theta,
                        int threshold, vector<Vec2f>* lines, vector<Vec2i>* contributions, int mode)
{
    vector<int> _sort_buf;

    // stage 2. find local maximums
    for(int r = 0; r < numrho; r++ )
        for(int n = 0; n < numangle; n++ )
        {
            int base = (n+1) * (numrho+2) + r+1;

            // Must be greater than or equal to threshold
            if(mode == THRESH_GT)
            {
                if( accum[base] > threshold &&
                        accum[base] > accum[base - 1] && accum[base] >= accum[base + 1] &&
                        accum[base] > accum[base - numrho - 2] && accum[base] >= accum[base + numrho + 2] )
                    _sort_buf.push_back(base);
            }

            // Must be equal to threshold
            else if (mode == THRESH_EQ)
            {
                if( accum[base] == threshold &&
                        accum[base] > accum[base - 1] && accum[base] >= accum[base + 1] &&
                        accum[base] > accum[base - numrho - 2] && accum[base] >= accum[base + numrho + 2] )
                    _sort_buf.push_back(base);
            }
        }

    // stage 3. sort the detected lines by accumulator value
    std::sort(_sort_buf.begin(), _sort_buf.end(), hough_cmp_gt(accum));

    // stage 4. store the lines in the output buffer
    double scale = 1./(numrho+2);
    for( auto iter = _sort_buf.begin(); iter != _sort_buf.end(); iter++)
    {
        LinePolar line;
        int idx = *iter;
        int n = cvFloor(idx*scale) - 1;
        int r = idx - (n+1)*(numrho+2) - 1;
        line.rho = (r - (numrho - 1)*0.5f) * rho;
        line.angle = static_cast<float>(min_theta) + n * theta;

        // store accumulator cell positions and lines
        contributions->push_back(Vec2i(r, n));
        lines->push_back(Vec2f(line.rho, line.angle));
    }
}

// Deletes the contributions denoted by (numrho, numangle) pairs in the
// passed vector from the Hough accumulator.
void deleteLineContributions (int* accumulator, int numangle, vector<Vec2i> contributions)
{
    for(auto iter = contributions.begin(); iter != contributions.end(); iter++)
    {
        Vec2f pos = *iter;
        int r = pos[0];
        int n = pos[1];

        accumulator[r * numangle + n]--;
    }
}
