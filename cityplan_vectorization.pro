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
    src/main.cpp \
    src/text_segmentation/areafilter.cpp \
    src/text_segmentation/auxiliary.cpp \
    src/text_segmentation/collineargroup.cpp \
    src/text_segmentation/collineargrouping.cpp \
    src/text_segmentation/collinearphrase.cpp \
    src/text_segmentation/collinearstring.cpp \
    src/text_segmentation/colorconversions.cpp \
    src/text_segmentation/connectedcomponent.cpp \
    src/text_segmentation/customhoughtransform.cpp \
    src/text_segmentation/statistics.cpp \
    src/text_segmentation/unionfind.cpp \
    src/text_segmentation/unionfindcomponents.cpp \
    src/vectorization/moore.cpp \
    src/vectorization/vectorize.cpp \
    src/vectorization/vectorline.cpp \
    src/vectorization/zhangsuen.cpp \
    src/vectorization/douglaspeucker.cpp \
    src/vectorization/iterative_linematching.cpp

HEADERS += \
    include/opencvincludes.hpp \
    include/vec2icompare.hpp \
    include/cairo/drm/cairo-drm-i915-private.h \
    include/cairo/drm/cairo-drm-i965-private.h \
    include/cairo/drm/cairo-drm-intel-brw-defines.h \
    include/cairo/drm/cairo-drm-intel-brw-eu.h \
    include/cairo/drm/cairo-drm-intel-brw-structs.h \
    include/cairo/drm/cairo-drm-intel-command-private.h \
    include/cairo/drm/cairo-drm-intel-ioctl-private.h \
    include/cairo/drm/cairo-drm-intel-private.h \
    include/cairo/drm/cairo-drm-ioctl-private.h \
    include/cairo/drm/cairo-drm-private.h \
    include/cairo/drm/cairo-drm-radeon-private.h \
    include/cairo/skia/cairo-skia-private.h \
    include/cairo/win32/cairo-win32-private.h \
    include/cairo/cairo-analysis-surface-private.h \
    include/cairo/cairo-arc-private.h \
    include/cairo/cairo-array-private.h \
    include/cairo/cairo-atomic-private.h \
    include/cairo/cairo-backend-private.h \
    include/cairo/cairo-beos.h \
    include/cairo/cairo-box-inline.h \
    include/cairo/cairo-boxes-private.h \
    include/cairo/cairo-cache-private.h \
    include/cairo/cairo-clip-inline.h \
    include/cairo/cairo-clip-private.h \
    include/cairo/cairo-cogl-context-private.h \
    include/cairo/cairo-cogl-gradient-private.h \
    include/cairo/cairo-cogl-private.h \
    include/cairo/cairo-cogl-utils-private.h \
    include/cairo/cairo-cogl.h \
    include/cairo/cairo-combsort-inline.h \
    include/cairo/cairo-compiler-private.h \
    include/cairo/cairo-composite-rectangles-private.h \
    include/cairo/cairo-compositor-private.h \
    include/cairo/cairo-contour-inline.h \
    include/cairo/cairo-contour-private.h \
    include/cairo/cairo-damage-private.h \
    include/cairo/cairo-default-context-private.h \
    include/cairo/cairo-deprecated.h \
    include/cairo/cairo-device-private.h \
    include/cairo/cairo-directfb.h \
    include/cairo/cairo-drm.h \
    include/cairo/cairo-error-inline.h \
    include/cairo/cairo-error-private.h \
    include/cairo/cairo-features.h \
    include/cairo/cairo-fixed-private.h \
    include/cairo/cairo-fixed-type-private.h \
    include/cairo/cairo-fontconfig-private.h \
    include/cairo/cairo-freed-pool-private.h \
    include/cairo/cairo-freelist-private.h \
    include/cairo/cairo-freelist-type-private.h \
    include/cairo/cairo-ft-private.h \
    include/cairo/cairo-ft.h \
    include/cairo/cairo-gl-dispatch-private.h \
    include/cairo/cairo-gl-ext-def-private.h \
    include/cairo/cairo-gl-gradient-private.h \
    include/cairo/cairo-gl-private.h \
    include/cairo/cairo-gl.h \
    include/cairo/cairo-gstate-private.h \
    include/cairo/cairo-hash-private.h \
    include/cairo/cairo-image-info-private.h \
    include/cairo/cairo-image-surface-inline.h \
    include/cairo/cairo-image-surface-private.h \
    include/cairo/cairo-line-inline.h \
    include/cairo/cairo-line-private.h \
    include/cairo/cairo-list-inline.h \
    include/cairo/cairo-list-private.h \
    include/cairo/cairo-malloc-private.h \
    include/cairo/cairo-mempool-private.h \
    include/cairo/cairo-mutex-impl-private.h \
    include/cairo/cairo-mutex-list-private.h \
    include/cairo/cairo-mutex-private.h \
    include/cairo/cairo-mutex-type-private.h \
    include/cairo/cairo-os2-private.h \
    include/cairo/cairo-os2.h \
    include/cairo/cairo-output-stream-private.h \
    include/cairo/cairo-paginated-private.h \
    include/cairo/cairo-paginated-surface-private.h \
    include/cairo/cairo-path-fixed-private.h \
    include/cairo/cairo-path-private.h \
    include/cairo/cairo-pattern-inline.h \
    include/cairo/cairo-pattern-private.h \
    include/cairo/cairo-pdf-operators-private.h \
    include/cairo/cairo-pdf-shading-private.h \
    include/cairo/cairo-pdf-surface-private.h \
    include/cairo/cairo-pdf.h \
    include/cairo/cairo-pixman-private.h \
    include/cairo/cairo-private.h \
    include/cairo/cairo-ps-surface-private.h \
    include/cairo/cairo-ps.h \
    include/cairo/cairo-qt.h \
    include/cairo/cairo-quartz-image.h \
    include/cairo/cairo-quartz-private.h \
    include/cairo/cairo-quartz.h \
    include/cairo/cairo-recording-surface-inline.h \
    include/cairo/cairo-recording-surface-private.h \
    include/cairo/cairo-reference-count-private.h \
    include/cairo/cairo-region-private.h \
    include/cairo/cairo-rtree-private.h \
    include/cairo/cairo-scaled-font-private.h \
    include/cairo/cairo-scaled-font-subsets-private.h \
    include/cairo/cairo-script-private.h \
    include/cairo/cairo-script.h \
    include/cairo/cairo-skia.h \
    include/cairo/cairo-slope-private.h \
    include/cairo/cairo-spans-compositor-private.h \
    include/cairo/cairo-spans-private.h \
    include/cairo/cairo-stroke-dash-private.h \
    include/cairo/cairo-surface-backend-private.h \
    include/cairo/cairo-surface-clipper-private.h \
    include/cairo/cairo-surface-fallback-private.h \
    include/cairo/cairo-surface-inline.h \
    include/cairo/cairo-surface-observer-inline.h \
    include/cairo/cairo-surface-observer-private.h \
    include/cairo/cairo-surface-offset-private.h \
    include/cairo/cairo-surface-private.h \
    include/cairo/cairo-surface-snapshot-inline.h \
    include/cairo/cairo-surface-snapshot-private.h \
    include/cairo/cairo-surface-subsurface-inline.h \
    include/cairo/cairo-surface-subsurface-private.h \
    include/cairo/cairo-surface-wrapper-private.h \
    include/cairo/cairo-svg-surface-private.h \
    include/cairo/cairo-svg.h \
    include/cairo/cairo-tee-surface-private.h \
    include/cairo/cairo-tee.h \
    include/cairo/cairo-time-private.h \
    include/cairo/cairo-traps-private.h \
    include/cairo/cairo-tristrip-private.h \
    include/cairo/cairo-truetype-subset-private.h \
    include/cairo/cairo-type1-private.h \
    include/cairo/cairo-type3-glyph-surface-private.h \
    include/cairo/cairo-types-private.h \
    include/cairo/cairo-user-font-private.h \
    include/cairo/cairo-version.h \
    include/cairo/cairo-vg.h \
    include/cairo/cairo-wideint-private.h \
    include/cairo/cairo-wideint-type-private.h \
    include/cairo/cairo-win32.h \
    include/cairo/cairo-xcb-private.h \
    include/cairo/cairo-xcb.h \
    include/cairo/cairo-xlib-private.h \
    include/cairo/cairo-xlib-surface-private.h \
    include/cairo/cairo-xlib-xrender-private.h \
    include/cairo/cairo-xlib-xrender.h \
    include/cairo/cairo-xlib.h \
    include/cairo/cairo-xml.h \
    include/cairo/cairo.h \
    include/cairo/cairoint.h \
    include/cairo/test-compositor-surface-private.h \
    include/cairo/test-compositor-surface.h \
    include/cairo/test-null-compositor-surface.h \
    include/cairo/test-paginated-surface.h \
    include/text_segmentation/areafilter.hpp \
    include/text_segmentation/auxiliary.hpp \
    include/text_segmentation/collineargroup.hpp \
    include/text_segmentation/collineargrouping.hpp \
    include/text_segmentation/collinearphrase.hpp \
    include/text_segmentation/collinearstring.hpp \
    include/text_segmentation/colorconversions.hpp \
    include/text_segmentation/connectedcomponent.hpp \
    include/text_segmentation/customhoughtransform.hpp \
    include/text_segmentation/statistics.hpp \
    include/text_segmentation/unionfind.hpp \
    include/text_segmentation/unionfindcomponents.hpp \
    include/vectorization/moore.hpp \
    include/vectorization/vectorize.hpp \
    include/vectorization/vectorline.hpp \
    include/vectorization/zhangsuen.hpp \
    include/vectorization/douglaspeucker.h \
    include/vectorization/iterative_linematching.hpp

INCLUDEPATH += "C:/opencv/build/include"

# openCV libraries
LIBS += -LC:/opencv/build/x86/mingw/bin \
            -lopencv_highgui300 \
            -lopencv_imgproc300 \
            -lopencv_core300 \
            -lopencv_imgcodecs300 \

# Cairo library; for generating .svg files
LIBS += -L"$$PWD/lib" -lcairo



