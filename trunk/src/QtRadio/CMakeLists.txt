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


#--------------------------------------------------------------
#
# Check for Ubuntu non standard installation
#
message ("qmake: ${QT_QMAKE_EXECUTABLE}")  

# get the path of the Qt header 
#
execute_process(
    COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_HEADERS
    OUTPUT_VARIABLE QT_INSTALL_HEADERS
    RESULT_VARIABLE rm_retval
)

# trim the last directory in path
string(REGEX REPLACE "(.*/)[^/]+$" "\\1" QT_INSTALL_HEADERS_UP ${QT_INSTALL_HEADERS})

# trim the end of line character
string (REGEX REPLACE "\n" ""  QT_INSTALL_HEADERS ${QT_INSTALL_HEADERS})

message("DIR INCLUDE: ${QT_INSTALL_HEADERS}")
message("DIR INSTALL HEADERS UP: ${QT_INSTALL_HEADERS_UP}")

# get the path of the Qt librarires 
#
execute_process(
    COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_LIBS
    OUTPUT_VARIABLE QT_INSTALL_LIBS
    RESULT_VARIABLE rm_retval
)
# trim the end of line character
string (REGEX REPLACE "\n" ""  QT_INSTALL_LIBS ${QT_INSTALL_LIBS})



# default for standard QtSDK and binary packages before U11.04
#
set (QT_ADDITIONAL_INCLUDE_PATH "")
set (QT_ADDITIONAL_INCLUDE_PATH2 "")
set (QT_ADDITIONAL_LDFLAG "QtMultimedia" )


# if there is a suspicious QtMultimediaKit (sibling of the standard Qt include path)
# we are hit an Ubuntu binary package (11.04)
#
if ( EXISTS ${QT_INSTALL_HEADERS_UP}QtMultimediaKit/QAudio )

    set (QT_ADDITIONAL_INCLUDE_PATH ${QT_INSTALL_HEADERS_UP}/QtMultimediaKit/ )
    set (QT_ADDITIONAL_LDFLAG QtMultimediaKit)
    set (QT_ADDITIONAL_INCLUDE_PATH2 ${QT_INSTALL_HEADERS_UP}/QtMobility/ )

else ( EXISTS ${QT_INSTALL_HEADERS_UP}QtMultimediaKit/QAudio )

    #set (QT_ADDITIONAL_INCLUDE_PATH ${QT_INSTALL_HEADERS} )
    set (QT_ADDITIONAL_INCLUDE_PATH ${QT_INSTALL_HEADERS}/QtMultimedia/ )
    set (QT_ADDITIONAL_LINK_DIR ${QT_INSTALL_LIBS})

endif ( EXISTS ${QT_INSTALL_HEADERS_UP}QtMultimediaKit/QAudio )


message("QT_ADDITIONAL_INCLUDE_PATH:  ${QT_ADDITIONAL_INCLUDE_PATH}")
message("QT_ADDITIONAL_INCLUDE_PATH2: ${QT_ADDITIONAL_INCLUDE_PATH2}")
message("QT_ADDITIONAL_LDFLAG: ${QT_ADDITIONAL_LDFLAG}")

#--------------------------------------------------------------

#
# Build
#
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${QT_MULTIMEDIA_INCLUDE_DIR}
    ${QT_QTNETWORK_INCLUDE_DIR}

    #
    # non standard include path in order to workaround 
    # Ubuntu binary packages strangeness 
    #
    ${QT_ADDITIONAL_INCLUDE_PATH}
    ${QT_ADDITIONAL_INCLUDE_PATH2}
)

link_directories(
    #
    # non standard library path in order to workaround 
    # Ubuntu binary packages strangeness 
    #
    ${QT_ADDITIONAL_LINK_DIR}
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

    #
    # additional library in order to workaround 
    # Ubuntu binary packages strangeness 
    #
    ${QT_ADDITIONAL_LDFLAG}
    codec2
)



INSTALL_TARGETS(/bin QtRadio)


