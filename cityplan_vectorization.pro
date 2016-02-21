QT += core
QT -= gui

#QMAKE_CXXFLAGS_DEBUG += -pg
#QMAKE_LFLAGS_DEBUG += -pg

TARGET = cityplan_vectorization
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

TEMPLATE = app

SOURCES += \
    src/areafilter.cpp \
    src/auxiliary.cpp \
    src/collineargrouping.cpp \
    src/colorconversions.cpp \
    src/connectedcomponent.cpp \
    src/customhoughtransform.cpp \
    src/main.cpp \
    src/unionfind.cpp \
    src/unionfindcomponents.cpp \
    src/zhangsuen.cpp \
    src/collinearstring.cpp \
    src/collineargroup.cpp \
    src/collinearphrase.cpp \
    src/statistics.cpp \
    src/vectorize.cpp

HEADERS += \
    include/areafilter.hpp \
    include/auxiliary.hpp \
    include/collineargrouping.hpp \
    include/colorconversions.hpp \
    include/connectedcomponent.hpp \
    include/customhoughtransform.hpp \
    include/opencvincludes.hpp \
    include/unionfind.hpp \
    include/unionfindcomponents.hpp \
    include/zhangsuen.hpp \
    include/vec2icompare.hpp \
    include/collinearstring.hpp \
    include/collineargroup.hpp \
    include/collinearphrase.hpp \
    include/statistics.hpp \
    include/vectorize.hpp

INCLUDEPATH += "C:/opencv/build/include"

# openCV libraries
LIBS += -LC:/opencv/build/x86/mingw/bin \
            -lopencv_highgui300 \
            -lopencv_imgproc300 \
            -lopencv_core300 \
            -lopencv_imgcodecs300 \

# Cairo library; for generating .svg files
LIBS += -L"$$PWD/lib" -lcairo



