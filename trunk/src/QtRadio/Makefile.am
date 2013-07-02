ACLOCAL_AMFLAGS = -I build-aux

# Manually list the files you're using from build-aux here.
# Note: Automake already takes care of the .mk and .m4 files you use.
include $(srcdir)/build-aux/autotroll.mk


QtRadio_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
QtRadio_CPPFLAGS = $(QT_ADDITIONAL_INCLUDE_PATH) $(QT_CPPFLAGS) $(AM_CPPFLAGS) -DGITREV="\"$(GITSTR)\""

QtRadio_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS) $(QT_ADDITIONAL_LDFLAG)
QtRadio_LDADD    = $(QT_LIBS) $(LDADD) $(QtRadio_LDFLAGS) -lssl -lcrypto

# For a program:
bin_PROGRAMS = QtRadio

BUILT_SOURCES=                  \
    ui_UI.h \
    ui_Configure.h \
    ui_Bandscope.h \
    ui_About.h \
    ui_Bookmark.h \
    ui_BookmarksDialog.h \
    ui_BookmarksEditDialog.h \
    ui_KeypadDialog.h \
    ui_vfo.h \
    ui_ctl.h \
    ui_servers.h \
    ui_EqualizerDialog.h \
About.moc.cpp                    \
Audio.moc.cpp                    \
Band.moc.cpp                     \
Bandscope.moc.cpp                \
BandStackEntry.moc.cpp           \
Bookmark.moc.cpp                 \
BookmarkDialog.moc.cpp           \
Bookmarks.moc.cpp                \
BookmarksDialog.moc.cpp          \
BookmarksEditDialog.moc.cpp      \
Configure.moc.cpp                \
Connection.moc.cpp               \
Filter.moc.cpp                   \
FiltersBase.moc.cpp              \
Filters.moc.cpp                  \
KeypadDialog.moc.cpp             \
Mode.moc.cpp                     \
Panadapter.moc.cpp               \
UI.moc.cpp                       \
vfo.moc.cpp                      \
Xvtr.moc.cpp \
smeter.moc.cpp \
ctl.moc.cpp \
rigctl.moc.cpp \
Audioinput.moc.cpp \
servers.moc.cpp \
G711A.moc.cpp \
RTP.moc.cpp \
hardware.moc.cpp \
hardware_hermes.moc.cpp \
hardware_perseus.moc.cpp \
hardware_sdriq.moc.cpp \
hardware_sdr1000.moc.cpp \
hardware_hiqsdr.moc.cpp \
hardware_rtlsdr.moc.cpp \
powermate.moc.cpp \
EqualizerDialog.moc.cpp
	
nodist_QtRadio_SOURCES = \
    ui_UI.h \
    ui_Configure.h \
    ui_Bandscope.h \
    ui_About.h \
    ui_Bookmark.h \
    ui_BookmarksDialog.h \
    ui_BookmarksEditDialog.h \
    ui_KeypadDialog.h \
    ui_vfo.h \
    ui_ctl.h \
    ui_EqualizerDialog.h \
About.moc.cpp                  \
Audio.moc.cpp                  \
Audioinput.moc.cpp		\
Band.moc.cpp                   \
Bandscope.moc.cpp              \
BandStackEntry.moc.cpp         \
Bookmark.moc.cpp               \
BookmarkDialog.moc.cpp         \
Bookmarks.moc.cpp              \
BookmarksDialog.moc.cpp        \
BookmarksEditDialog.moc.cpp    \
Configure.moc.cpp              \
Connection.moc.cpp             \
Filter.moc.cpp                 \
FiltersBase.moc.cpp            \
Filters.moc.cpp                \
KeypadDialog.moc.cpp           \
Mode.moc.cpp                   \
Panadapter.moc.cpp             \
UI.moc.cpp                     \
vfo.moc.cpp                    \
Xvtr.moc.cpp                   \
smeter.moc.cpp                 \
ctl.moc.cpp \
rigctl.moc.cpp \
servers.moc.cpp \
G711A.moc.cpp \
RTP.moc.cpp \
hardware.moc.cpp \
hardware_hermes.moc.cpp \
hardware_perseus.moc.cpp \
hardware_sdriq.moc.cpp \
hardware_sdr1000.moc.cpp \
hardware_hiqsdr.moc.cpp \
hardware_rtlsdr.moc.cpp \
powermate.moc.cpp \
EqualizerDialog.moc.cpp


QtRadio_SOURCES = \
About.cpp      BandStackEntry.cpp       Buffer.cpp       DIGUFilters.cpp  Frequency.cpp      Mode.cpp        EqualizerDialog.cpp  \
AMFilters.cpp  Bookmark.cpp             Configure.cpp    DSBFilters.cpp   FrequencyInfo.cpp  SAMFilters.cpp  Xvtr.cpp       \
Audio.cpp      BookmarkDialog.cpp       Connection.cpp   Filter.cpp       KeypadDialog.cpp   Panadapter.cpp  XvtrEntry.cpp  \
Band.cpp       Bookmarks.cpp            CWLFilters.cpp   FiltersBase.cpp  LSBFilters.cpp     UI.cpp                         \
BandLimit.cpp  BookmarksDialog.cpp      CWUFilters.cpp   Filters.cpp      main.cpp           USBFilters.cpp                 \
Bandscope.cpp  BookmarksEditDialog.cpp  DIGLFilters.cpp  FMNFilters.cpp   Meter.cpp          vfo.cpp         smeter.cpp     \
ctl.cpp rigctl.cpp\
Audioinput.cpp \
servers.cpp\
G711A.cpp RTP.cpp hardware.cpp hardware_hermes.cpp hardware_perseus.cpp hardware_sdriq.cpp hardware_sdr1000.cpp hardware_hiqsdr.cpp hardware_rtlsdr.cpp powermate.cpp \
About.h      Bandscope.h        BookmarksEditDialog.h  Connection.h   DSBFilters.h   Frequency.h      Mode.h        vfo.h        \
AMFilters.h  BandStackEntry.h   Bookmarks.h            CWLFilters.h   Filter.h       FrequencyInfo.h  SAMFilters.h  EqualizerDialog.h \
Audio.h      BookmarkDialog.h   Buffer.h               CWUFilters.h   FiltersBase.h  KeypadDialog.h   Panadapter.h    XvtrEntry.h  \
Band.h       Bookmark.h         codec2.h               DIGLFilters.h  Filters.h      LSBFilters.h     UI.h          Xvtr.h       \
BandLimit.h  BookmarksDialog.h  Configure.h            DIGUFilters.h  FMNFilters.h   Meter.h          USBFilters.h  smeter.h \
ctl.h rigctl.h\
Audioinput.h \
servers.h \
G711A.h RTP.h \
cusdr_queue.h \
hardware.h \
hardware_hermes.h \
hardware_perseus.h \
hardware_sdriq.h \
hardware_sdr1000.h \
hardware_hiqsdr.h \
hardware_rtlsdr.h \
powermate.h \
About.ui  Bandscope.ui  BookmarksDialog.ui  BookmarksEditDialog.ui  Bookmark.ui  Configure.ui  KeypadDialog.ui  UI.ui  vfo.ui \
ctl.ui servers.ui EqualizerDialog.ui


CLEANFILES =                  \
    ui_UI.h \
    ui_Configure.h \
    ui_Bandscope.h \
    ui_About.h \
    ui_Bookmark.h \
    ui_BookmarksDialog.h \
    ui_BookmarksEditDialog.h \
    ui_KeypadDialog.h \
    ui_vfo.h \
    ui_ctl.h \
    ui_servers.h \
    ui_EqualizerDialog.h \
About.moc.cpp                    \
Audio.moc.cpp                    \
Band.moc.cpp                     \
Bandscope.moc.cpp                \
BandStackEntry.moc.cpp           \
Bookmark.moc.cpp                 \
BookmarkDialog.moc.cpp           \
Bookmarks.moc.cpp                \
BookmarksDialog.moc.cpp          \
BookmarksEditDialog.moc.cpp      \
Configure.moc.cpp                \
Connection.moc.cpp               \
Filter.moc.cpp                   \
FiltersBase.moc.cpp              \
Filters.moc.cpp                  \
KeypadDialog.moc.cpp             \
Mode.moc.cpp                     \
Panadapter.moc.cpp               \
UI.moc.cpp                       \
vfo.moc.cpp                      \
Xvtr.moc.cpp                     \
smeter.moc.cpp                   \
ctl.moc.cpp \
rigctl.moc.cpp \
aux.moc.cpp			\
Audioinput.moc.cpp		\
servers.moc.cpp \
G711A.moc.cpp \
RTP.moc.cpp \
hardware.moc.cpp \
hardware_hermes.moc.cpp \
hardware_perseus.moc.cpp \
hardware_sdriq.moc.cpp \
hardware_sdr1000.moc.cpp \
hardware_hiqsdr.moc.cpp \
hardware_rtlsdr.moc.cpp \
powermate.moc.cpp \
EqualizerDialog.moc.cpp

#
# auxiliary services, git string, icons
#


desktop_in_files = QtRadio.desktop.in
desktopdir = $(datadir)/applications
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)



EXTRA_DIST = GITSTR  icons $(desktop_in_files) $(desktop_DATA)

pixmapdir = $(datadir)/icons
pixmap_DATA = $(srcdir)/icons/QtRadio.png


UPDATE_DESKTOP = \
	if which update-desktop-database>/dev/null 2>&1 ; then \
		update-desktop-database; \
	fi

install-data-hook:
	xdg-desktop-menu install --mode user --novendor $(srcdir)/QtRadio.desktop

uninstall-hook:
	xdg-desktop-menu uninstall --mode user --novendor QtRadio.desktop

