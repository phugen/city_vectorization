#ifndef CITY_VECTORIZATION_H
#define CITY_VECTORIZATION_H

#include <QtWidgets/QMainWindow>

#include "ui_city_vectorization.h"

#include <QMainWindow>
#include <QFileDialog>
#include <QPushButton>
#include <QBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QStatusBar>

#include "ImageReader.hpp"
#include "QtOpencvCore.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

class city_vectorization : public QMainWindow
{
	Q_OBJECT

public:
	city_vectorization(QWidget *parent = 0);
	~city_vectorization();

	private slots:

	/* Funktionen werden ausgelöst, wenn auf den entsprechenden Button geklickt wird */
	void on_pbOpenImage_clicked();
	void on_pbTestAction_clicked();

private:
	Ui::city_vectorizationClass ui;

	/* storage */

	/* GUI Komonete */

	//QPushButton *pbOpenImage;


	/*****************************************/

	/* Originalbild */
	cv::Mat         originalImage;
	/* Eventuel weitere Klassenattribute */


	/* Methode initialisiert die UI */

	/* Methoden aktivieren bzw. deaktivieren die UI */

};

#endif // CITY_VECTORIZATION_H
