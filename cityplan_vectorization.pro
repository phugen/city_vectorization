QT += core
QT -= gui

TARGET = cityplan_vectorization
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    connectedcomponent.cpp \
    colorconversions.cpp \
    unionfind.cpp \
    customhoughtransform.cpp

HEADERS += \
    connectedcomponent.hpp \
    colorconversions.hpp \
    unionfind.hpp \
    opencvincludes.hpp \
    customhoughtransform.hpp

INCLUDEPATH += "C:/opencv/build/include"

LIBS += -LC:/opencv/build/x86/mingw/bin \
            -lopencv_highgui300 \
            -lopencv_imgproc300 \
            -lopencv_core300 \
            -lopencv_imgcodecs300 \

CONFIG += c++11
