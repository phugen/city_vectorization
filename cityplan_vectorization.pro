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
    customhoughtransform.cpp \
    src/colorconversions.cpp \
    src/connectedcomponent.cpp \
    src/customhoughtransform.cpp \
    src/main.cpp \
    src/auxiliary.cpp \
    src/collineargrouping.cpp \
    src/areafilter.cpp \
    src/unionfindcomponents.cpp \
    src/unionfind.cpp

HEADERS += \
    connectedcomponent.hpp \
    colorconversions.hpp \
    unionfind.hpp \
    opencvincludes.hpp \
    customhoughtransform.hpp \
    header/colorconversions.hpp \
    header/connectedcomponent.hpp \
    header/customhoughtransform.hpp \
    header/medoidHough.hpp \
    header/opencvincludes.hpp \
    header/unionfind.hpp \
    include/colorconversions.hpp \
    include/connectedcomponent.hpp \
    include/customhoughtransform.hpp \
    include/opencvincludes.hpp \
    include/auxiliary.hpp \
    include/collineargrouping.hpp \
    include/areafilter.hpp \
    include/unionfindcomponents.hpp \
    include/unionfind.hpp

INCLUDEPATH += "C:/opencv/build/include"

LIBS += -LC:/opencv/build/x86/mingw/bin \
            -lopencv_highgui300 \
            -lopencv_imgproc300 \
            -lopencv_core300 \
            -lopencv_imgcodecs300 \

CONFIG += c++11
