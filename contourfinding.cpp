

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "contourfinding.hpp"

using namespace cv;
using namespace std;


int thresh = 100;
int max_thresh = 255;
RNG rng(12345);
const String SOURCEWINDOWNAME = "Source";
const String OUTPUTWINDOWNAME = "Contours";
void thresh_callback(int, void*);
cv::Mat src_gray;
vector<vector<Point> > contours;	//array of boundary points (x,y) for each contour
vector<Vec4i> hierarchy;

void contourfinding::execute(Mat source) {

	cvtColor(source, src_gray, COLOR_BGR2GRAY);
	blur(src_gray, src_gray, Size(3, 3));

	namedWindow(SOURCEWINDOWNAME, WINDOW_AUTOSIZE);
	imshow(SOURCEWINDOWNAME, source);
	createTrackbar(" Canny thresh:", SOURCEWINDOWNAME, &thresh, max_thresh, thresh_callback);
	thresh_callback(0, 0);
	waitKey(0);
}

vector<vector<Point> > contourfinding::getContours() {
	if (!contours.empty())
		return contours;
}

vector<Vec4i> contourfinding::getHierarchy() {
	if (!hierarchy.empty())
		return hierarchy;
}


void thresh_callback(int, void*)		//first parameter: trackbar position, second parameter: userdata given to onChange method (not needed here)
{
	Mat canny_output;
	
	Canny(src_gray, canny_output, thresh, thresh * 2, 3);
	findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));			//overwrites source image!!
	Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
	for (size_t i = 0; i< contours.size(); i++)																//draw ALL contours
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(drawing, contours, (int)i, color, 2, 8, hierarchy, 0, Point());
	}
	namedWindow(OUTPUTWINDOWNAME, WINDOW_AUTOSIZE);
	imshow(OUTPUTWINDOWNAME, drawing);

	canny_output.release();
	drawing.release();
}

