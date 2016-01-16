#include "city_vectorization.h"

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
