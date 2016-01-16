/********************************************************************************
** Form generated from reading UI file 'city_vectorization.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CITY_VECTORIZATION_H
#define UI_CITY_VECTORIZATION_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_city_vectorizationClass
{
public:
    QWidget *centralWidget;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    QPushButton *pbOpenImage;
    QPushButton *pbTestAction;
    QPushButton *pushButton;
    QSpacerItem *verticalSpacer;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *city_vectorizationClass)
    {
        if (city_vectorizationClass->objectName().isEmpty())
            city_vectorizationClass->setObjectName(QStringLiteral("city_vectorizationClass"));
        city_vectorizationClass->resize(129, 281);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(city_vectorizationClass->sizePolicy().hasHeightForWidth());
        city_vectorizationClass->setSizePolicy(sizePolicy);
        city_vectorizationClass->setMinimumSize(QSize(129, 281));
        city_vectorizationClass->setMaximumSize(QSize(129, 281));
        centralWidget = new QWidget(city_vectorizationClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(centralWidget->sizePolicy().hasHeightForWidth());
        centralWidget->setSizePolicy(sizePolicy1);
        centralWidget->setMaximumSize(QSize(129, 281));
        horizontalLayoutWidget = new QWidget(centralWidget);
        horizontalLayoutWidget->setObjectName(QStringLiteral("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(0, 0, 131, 261));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        pbOpenImage = new QPushButton(horizontalLayoutWidget);
        pbOpenImage->setObjectName(QStringLiteral("pbOpenImage"));

        verticalLayout->addWidget(pbOpenImage);

        pbTestAction = new QPushButton(horizontalLayoutWidget);
        pbTestAction->setObjectName(QStringLiteral("pbTestAction"));

        verticalLayout->addWidget(pbTestAction);

        pushButton = new QPushButton(horizontalLayoutWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));

        verticalLayout->addWidget(pushButton);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        horizontalLayout->addLayout(verticalLayout);

        city_vectorizationClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(city_vectorizationClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 129, 21));
        city_vectorizationClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(city_vectorizationClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        city_vectorizationClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(city_vectorizationClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        city_vectorizationClass->setStatusBar(statusBar);

        retranslateUi(city_vectorizationClass);

        QMetaObject::connectSlotsByName(city_vectorizationClass);
    } // setupUi

    void retranslateUi(QMainWindow *city_vectorizationClass)
    {
        city_vectorizationClass->setWindowTitle(QApplication::translate("city_vectorizationClass", "city_vectorization", 0));
        pbOpenImage->setText(QApplication::translate("city_vectorizationClass", "Open Image", 0));
        pbTestAction->setText(QApplication::translate("city_vectorizationClass", "Action", 0));
        pushButton->setText(QApplication::translate("city_vectorizationClass", "PushButton", 0));
    } // retranslateUi

};

namespace Ui {
    class city_vectorizationClass: public Ui_city_vectorizationClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CITY_VECTORIZATION_H
