project(QtRadio)
cmake_minimum_required(VERSION 2.8.0)

set(CMAKE_VERBOSE_MAKEFILE OFF)

# Versions
set(QtRadio_VERSION_MAJOR 1)
set(QtRadio_VERSION_MINOR 6)
set(QtRadio_VERSION_PATCH 1)
set(QtRadio_VERSION ${QtRadio_VERSION_MAJOR}.${QtRadio_VERSION_MINOR}.${QtRadio_VERSION_PATCH})


# Dependencies
find_package(PkgConfig REQUIRED)
find_package(Qt4 REQUIRED)



# Source
set(QtRadio_SRCS
About.cpp
AMFilters.cpp
Audio.cpp
Band.cpp
BandLimit.cpp
Bandscope.cpp
BandStackEntry.cpp
Bookmark.cpp
BookmarkDialog.cpp
Bookmarks.cpp
BookmarksDialog.cpp
BookmarksEditDialog.cpp
Buffer.cpp
Configure.cpp
Connection.cpp
ctl.cpp
CWLFilters.cpp
CWUFilters.cpp
DIGLFilters.cpp
DIGUFilters.cpp
DSBFilters.cpp
Filter.cpp
FiltersBase.cpp
Filters.cpp
FMNFilters.cpp
Frequency.cpp
FrequencyInfo.cpp
KeypadDialog.cpp
LSBFilters.cpp
main.cpp
Meter.cpp
Mode.cpp
rigctl.cpp
SAMFilters.cpp
smeter.cpp
Spectrum.cpp
UI.cpp
USBFilters.cpp
vfo.cpp
Waterfall.cpp
Xvtr.cpp
XvtrEntry.cpp
)

set (QtRadio_INC
About.h
AMFilters.h
Audio.h
Band.h
BandLimit.h
Bandscope.h
BandStackEntry.h
BookmarkDialog.h
Bookmark.h
BookmarksDialog.h
BookmarksEditDialog.h
Bookmarks.h
Buffer.h
codec2.h
Configure.h
Connection.h
ctl.h
CWLFilters.h
CWUFilters.h
DIGLFilters.h
DIGUFilters.h
DSBFilters.h
Filter.h
FiltersBase.h
Filters.h
FMNFilters.h
Frequency.h
FrequencyInfo.h
KeypadDialog.h
LSBFilters.h
Meter.h
Mode.h
rigctl.h
SAMFilters.h
smeter.h
Spectrum.h
UI.h
USBFilters.h
vfo.h
Waterfall.h
XvtrEntry.h
Xvtr.h
)
    
set( QtRadio_UIS
About.ui
Bandscope.ui
BookmarksDialog.ui
BookmarksEditDialog.ui
Bookmark.ui
Configure.ui
ctl.ui
KeypadDialog.ui
UI.ui
vfo.ui

)

#SET(QT_USE_QTOPENGL TRUE)
#SET(QT_USE_QTNETWORK TRUE)


# Build
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${QT_MULTIMEDIA_INCLUDE_DIR}
    ${QT_QTNETWORK_INCLUDE_DIR}
    /usr/include/QtMultimediaKit
    /usr/include/QtMobility
    )
    
link_directories(
/home/andrew/QtSDK/Desktop/Qt/473/gcc/lib/
/usr/lib/
/usr/local/lib/
)

QT4_WRAP_UI( QtRadio_UI_HDRS ${QtRadio_UIS} )


qt4_wrap_cpp (QtRadio_MOCS ${QtRadio_INC} )

add_executable(QtRadio ${QtRadio_SRCS} ${QtRadio_UI_HDRS} ${QtRadio_MOCS} )


target_link_libraries ( QtRadio
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTOPENGL_LIBRARY}
    ${QT_MULTIMEDIA_LIBRARY}
    QtMultimedia
    codec2
)





