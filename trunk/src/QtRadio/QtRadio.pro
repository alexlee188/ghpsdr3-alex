#-------------------------------------------------
#
# Project created by QtCreator 2010-07-05T10:00:09
#
#-------------------------------------------------



#-------------------------------------------------
# If using the Nokia Qt SDK set _UsingSDK to true
# or if using Ubuntu repo set _UsingSDK to false
#-------------------------------------------------
_UsingSDK = true

greaterThan(QT_MAJOR_VERSION, 4) {
    message("Using Qt5")
    QT       += core gui widgets multimedia mobility

    INCLUDEPATH += /opt/qt5/include
    INCLUDEPATH += /opt/qt5/include/QtMultimedia
    INCLUDEPATH += /opt/qt5/include/QtNetwork

} else {
    $$_UsingSDK {
        message("Using the Nokia Qt SDK installation")
        QT       += core gui network multimedia mobility
        CONFIG   += mobility
        MOBILITY += multimedia
    } else {
        message("Using the Ubuntu Qt Creator installation")
        CONFIG   += mobility
        MOBILITY += multimedia
        INCLUDEPATH += /usr/include/QtMobility
        INCLUDEPATH += /usr/include/QtMultimediaKit
    }
}




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
    RTP.h \
    cusdr_queue.h

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

