#ifndef CONTOURFINDING_HPP
#define CONTOURFINDING_HPP

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

class contourfinding {
public: 
	static void execute(cv::Mat source);
	std::vector<std::vector<cv::Point> > getContours();
	std::vector<cv::Vec4i> getHierarchy();

private:
	
};
#endif