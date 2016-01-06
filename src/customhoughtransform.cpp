#include "include/customhoughtransform.hpp"
#include <iostream>

using namespace std;
using namespace cv;

// comparator for hough transform
struct hough_cmp_gt
{
    hough_cmp_gt(std::vector<int>* _aux) : aux(_aux) {}
    bool operator()(int l1, int l2) const
    {
        return aux[l1] > aux[l2] || (aux[l1] == aux[l2] && l1 < l2);
    }
    std::vector<int>* aux;
};



// Custom version of the Hough transformation algorithm for line detection.
// Allows access to the accumulator used in the Hough transformation.
// (Code based on the original openCV code)
void HoughLinesCustom( const Mat& img, float rho, float theta,
                        int threshold, std::vector<Vec2f>& lines, int linesMax,
                        double min_theta, double max_theta, int* accum)
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

    std::vector<int> _sort_buf;
    AutoBuffer<float> _tabSin(numangle);
    AutoBuffer<float> _tabCos(numangle);
    float *tabSin = _tabSin, *tabCos = _tabCos;

    float ang = static_cast<float>(min_theta);
    for(int n = 0; n < numangle; ang += theta, n++ )
    {
        tabSin[n] = (float)(sin((double)ang) * irho);
        tabCos[n] = (float)(cos((double)ang) * irho);
    }

    // fill accumulator
    for( i = 0; i < height; i++ )
        for( j = 0; j < width; j++ )
        {
            if( image[i * step + j] != 0 )
                for(int n = 0; n < numangle; n++ )
                {
                    int r = cvRound( j * tabCos[n] + i * tabSin[n] );
                    r += (numrho - 1) / 2;
                    accum[(n+1) * (numrho+2) + r+1]++;
                }
        }

    // find local maximums
    for(int r = 0; r < numrho; r++ )
        for(int n = 0; n < numangle; n++ )
        {
            int base = (n+1) * (numrho+2) + r+1;
            if( accum[base] > threshold &&
                accum[base] > accum[base - 1] && accum[base] >= accum[base + 1] &&
                accum[base] > accum[base - numrho - 2] && accum[base] >= accum[base + numrho + 2] )
                _sort_buf.push_back(base);
        }

    // stage 3. sort the detected lines by accumulator value
    //std::sort(_sort_buf.begin(), _sort_buf.end(), hough_cmp_gt(*accum));

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
    }
}

/*
    // find maximum and minimum value
    // for calculating brightness relative to accum value range
    int minVal = threshold;
    int maxVal = 0;

    for(int i = 0; i < numrho * numangle; i++)
    {
        maxVal = max(maxVal, accumulator[i]);
    }

    // color all pixels black
    for(int i = 0; i < height; i++)
        for(int j = 0; j < width; j++)
            showHough.at<uchar>(i, j) = 0;

    // display accumulator values mapped back to original image
    /*for(int i = 0; i < height; i++)
        for(int j = 0; j < width; j++)
            if(canny.at<uchar>(i, j) != 0) // if pixel is not an edge
                for(int n = 0; n < numangle; n++)
                {
                    int rho = cvRound (j * tabCos[n] + i * tabSin[n]);
                    rho += (numrho - 1) / 2;
                    int bin = n * numrho + rho;

                    if(accumulator[bin] > threshold)
                    {
                        //cout << accumulator[bin] << "\n";
                        float brightness = ((float) accumulator[bin] / (float) maxVal) * 255.0;
                        showHough.at<uchar>(i, j) = brightness;
                    }
                }

}*/
