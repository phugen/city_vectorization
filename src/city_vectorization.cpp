#include "city_vectorization.h"

//INSERT from OLD main
#include "include/opencvincludes.hpp"
#include "include/auxiliary.hpp"
#include "include/unionfindcomponents.hpp"
#include "include/areafilter.hpp"
#include "include/collineargrouping.hpp"
#include "include/zhangsuen.hpp"

using namespace std;
using namespace cv;
//END insert

#include <iostream>
#include <algorithm>

city_vectorization::city_vectorization(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
}

city_vectorization::~city_vectorization()
{
	/* lösche die UI Komponeten */
	

	/* schließe alle offenen Fenster */
	cv::destroyAllWindows();
}


/* Methode oeffnet ein Bild und zeigt es in einem separaten Fenster an */
void city_vectorization::on_pbOpenImage_clicked()
{
	/* oeffne Bild mit Hilfe eines Dateidialogs */
	QString imagePath = QFileDialog::getOpenFileName(this, "Open Image...", QString(), QString("Images *.png *.jpg *.tiff *.tif"));

	/* wenn ein gueltiger Dateipfad angegeben worden ist... */
	if (!imagePath.isNull() && !imagePath.isEmpty())
	{
		/* ...lese das Bild ein */
		cv::Mat img = ImageReader::readImage(QtOpencvCore::qstr2str(imagePath));

		/* wenn das Bild erfolgreich eingelesen worden ist... */
		if (!img.empty())
		{
			/* merke das Originalbild */
			originalImage = img;

			/* zeige das Originalbild in einem separaten Fenster an */
			cv::imshow("Original Image", originalImage);

		}
		else
		{

		}
	}
}

void city_vectorization::on_pbTestAction_clicked()
{
	
}

int city_vectorization::on_pbOldMain_clicked()
{
	Mat original, *output;
	//vector<ConnectedComponent> components;

	//

	//original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/CV_sample_schwer_2.png", CV_LOAD_IMAGE_COLOR);
	////original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/peter.png", CV_LOAD_IMAGE_COLOR);
	////original = imread("C:/Users/shk/Desktop/qtProj/cityplan_vectorization/CV_sample_leicht.png", CV_LOAD_IMAGE_COLOR);


	//if (!original.data)
	//{
	//	cout << "The image couldn't be loaded. Maybe the file name was wrong?\n";
	//	return -1;
	//}

	//output = new Mat(original.rows, original.cols, CV_8U); // output matrix
	//vector<Point2i> corners;
	//vector<Vec2i> vcorners;


	//// BGR format
	//// Thresholds need testing with unscaled images to avoid
	//// artifacts artificially increasing the needed threshold
	////
	//// TODO: Slider adjustment for black layer
	//Vec3b thresholds = Vec3b(180, 180, 180);

	//getBlackLayer(thresholds, original, output);
	//unionFindComponents(*output, &components);
	//areaFilter(&components, 10);
	//collinearGrouping(*output, &components);

	///*bitwise_not(*output, *output); // algorithm expects binary picture with black background
	//// find corners; find a way to describe min distance parameter dynamically if possible!
	////thinning(*output); // skeletonize image (1px lines only)
	//goodFeaturesToTrack(*output, corners, output->cols*output->rows, 0.1, 0 );
	//Mat reconstructed = Mat::zeros(original.rows, original.cols, original.type());
	//reconstructed = Scalar(255, 255, 255); // for showing reconstructed polygons
	//Mat cornerMat = *output; // for showing circles around detected corners
	//cvtColor(cornerMat, cornerMat,CV_GRAY2RGB);
	//bitwise_not(cornerMat, cornerMat);
	//// show corners
	//for(size_t i = 0; i < corners.size(); i++)
	//{
	//cout << "CORNER: " << corners[i] << "\n";
	////circle(cornerMat, corners[i], 10, Scalar(0, 255, 0), 1);
	//rectangle(cornerMat, corners[i], corners[i], Scalar(0, 0, 255), 3, 8, 0);
	//}
	//namedWindow("CORNERS", WINDOW_AUTOSIZE);
	//imshow("CORNERS", cornerMat);
	//// find reachable corners for every point
	//bitwise_not(*output, *output);
	//vcorners = pointToVec(corners);
	//for(auto iter = vcorners.begin(); iter != vcorners.end(); iter++)
	//{
	//Vec2i current = *iter;
	//vector<Vec2i> currNeigh = getNearestCorners(vcorners, current, output, &reconstructed);
	//}
	//namedWindow("RECONSTRUCTED", WINDOW_AUTOSIZE);
	//imshow("RECONSTRUCTED", reconstructed);*/

	////vector<Vec2f> line (0,0);
	////line.insert(line.begin(), Vec2f(50, 0.785398));
	////drawLines(line, output, Scalar(0, 255, 0));

	////namedWindow("linetest", WINDOW_AUTOSIZE);
	////imshow("linetest", *output);


	waitKey(0);

	return 0;
}