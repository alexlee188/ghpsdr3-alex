#-------------------------------------------------
#
# Project created by QtCreator 2010-07-05T10:00:09
#
#-------------------------------------------------

# Comment line below when using QtSDK, uncomment for  Ubuntu repository ver.
QT       += core gui network multimedia mobility

# Uncomment 2 lines below when using QtSDK, comment for  Ubuntu repository ver.
CONFIG	+= mobility
MOBILITY += multimedia

LIBS += -lQtOpenCLGL -lQtOpenCL -lQtOpenGL
macx:!opencl_configure {
    LIBS += -framework OpenCL
}
win32 {
    !isEmpty(QMAKE_INCDIR_OPENCL) {
        QMAKE_CXXFLAGS += -I$$QMAKE_INCDIR_OPENCL
    }
    !isEmpty(QMAKE_LIBDIR_OPENCL) {
        LIBS += -L$$QMAKE_LIBDIR_OPENCL
    }
    !isEmpty(QMAKE_LIBS_OPENCL) {
        LIBS += $$QMAKE_LIBS_OPENCL
    } else {
        LIBS += -lOpenCL
    }
}
QT += opengl
no_cl_gl {
    DEFINES += QT_NO_CL_OPENGL
}



# Comment 2 lines below when using QtSDK, uncomment for  Ubuntu repository ver.
INCLUDEPATH += /usr/include/QtMobility
INCLUDEPATH += /usr/include/QtMultimediaKit

TARGET = QtRadio
TEMPLATE = app

SOURCES += main.cpp\
    Waterfall.cpp \
    USBFilters.cpp \
    UI.cpp \
    Spectrum.cpp \
    SAMFilters.cpp \
    Mode.cpp \
    LSBFilters.cpp \
    FMNFilters.cpp \
    FiltersBase.cpp \
    Filters.cpp \
    Filter.cpp \
    DSBFilters.cpp \
    DIGUFilters.cpp \
    DIGLFilters.cpp \
    CWUFilters.cpp \
    CWLFilters.cpp \
    Connection.cpp \
    Configure.cpp \
    BandStackEntry.cpp \
    Band.cpp \
    Audio.cpp \
    AMFilters.cpp \
    BandLimit.cpp \
    FrequencyInfo.cpp \
    Frequency.cpp \
    Meter.cpp \
    Bandscope.cpp \
    About.cpp \
    Buffer.cpp \
    Bookmark.cpp \
    BookmarkDialog.cpp \
    BookmarksDialog.cpp \
    BookmarksEditDialog.cpp \
    Xvtr.cpp \
    XvtrEntry.cpp \
    Bookmarks.cpp \
    KeypadDialog.cpp \
    smeter.cpp \
    rigctl.cpp \
    vfo.cpp \
    ctl.cpp \
    Audioinput.cpp\
    servers.cpp \
    G711A.cpp \
    RTP.cpp


HEADERS  += \ 
    Waterfall.h \
    USBFilters.h \
    UI.h \
    Spectrum.h \
    SAMFilters.h \
    Mode.h \
    LSBFilters.h \
    FMNFilters.h \
    FiltersBase.h \
    Filters.h \
    Filter.h \
    DSBFilters.h \
    DIGUFilters.h \
    DIGLFilters.h \
    CWUFilters.h \
    CWLFilters.h \
    Connection.h \
    Configure.h \
    BandStackEntry.h \
    Band.h \
    Audio.h \
    AMFilters.h \
    BandLimit.h \
    FrequencyInfo.h \
    Frequency.h \
    Meter.h \
    Bandscope.h \
    About.h \
    Buffer.h \
    Bookmark.h \
    BookmarkDialog.h \
    BookmarksDialog.h \
    BookmarksEditDialog.h \
    Xvtr.h \
    XvtrEntry.h \
    Bookmarks.h \
    KeypadDialog.h \
    codec2.h \
    smeter.h \
    rigctl.h \
    vfo.h \
    ctl.h \
    Audioinput.h \
    servers.h \
    G711A.h \
    RTP.h

FORMS    += \   
    UI.ui \
    Configure.ui \
    Bandscope.ui \
    About.ui \
    Bookmark.ui \
    BookmarksDialog.ui \
    BookmarksEditDialog.ui \
    KeypadDialog.ui \
    vfo.ui \
    ctl.ui \
    servers.ui

OTHER_FILES +=

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../usr/lib/release/ 
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../usr/lib/debug/ 
else:symbian: LIBS += -lcodec2 -lsamplerate
else:unix: LIBS += -lcodec2 -lsamplerate -lortp

