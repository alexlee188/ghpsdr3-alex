#-------------------------------------------------
#
# Project created by QtCreator 2010-07-05T10:00:09
#
#-------------------------------------------------



#-------------------------------------------------
# If using the Nokia Qt SDK set _UsingSDK to true
# or if using Ubuntu repo set _UsingSDK to false
#-------------------------------------------------

_UsingSDK = false

greaterThan(QT_MAJOR_VERSION, 4) {
    message("Using Qt5")
    QT       += core gui widgets multimedia


#    INCLUDEPATH += /opt/qt5/include
#    INCLUDEPATH += /opt/qt5/include/QtMultimedia
#    INCLUDEPATH += /opt/qt5/include/QtNetwork

    #KD0NUZ OSX
    INCLUDEPATH += /usr/local/include

    INCLUDEPATH += /usr/local/Qt-5.0.2/include/QtCore
    INCLUDEPATH += /usr/local/Qt-5.0.2/include/QtGui
    INCLUDEPATH += /usr/local/Qt-5.0.2/include/QtWidgets
    INCLUDEPATH += /usr/local/Qt-5.0.2/include/QtMultimedia
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
    USBFilters.cpp \
    UI.cpp \
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
    RTP.cpp \
    hardware.cpp\
    powermate.cpp \
    hardware_sdr1000.cpp \
    calc.cpp \
    EqualizerDialog.cpp \
    hardware_sdriq.cpp \
    hardware_rtlsdr.cpp \
    hardware_perseus.cpp \
    hardware_hiqsdr.cpp \
    hardware_hermes.cpp \
    Panadapter.cpp


HEADERS  += \ 
    USBFilters.h \
    UI.h \
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
    cusdr_queue.h \
    hardware.h\
    powermate.h \
    hardware_sdr1000.h \
    hardware_sdr1000.h \
    calc.h \
    EqualizerDialog.h \
    hardware_sdriq.h \
    hardware_perseus.h \
    hardware_hiqsdr.h \
    hardware_hermes.h \
    hardware_rtlsdr.h \
    Panadapter.h

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
    servers.ui \
    EqualizerDialog.ui

OTHER_FILES +=

LIBS += -L/usr/local/lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../usr/lib/release/ 
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../usr/lib/debug/ 
else:symbian: LIBS += -lcodec2 -lsamplerate
else:unix: LIBS += -lcodec2 -lsamplerate -lortp

