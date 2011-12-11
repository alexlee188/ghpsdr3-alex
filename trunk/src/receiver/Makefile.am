

bin_PROGRAMS = ghpsdr

ghpsdr_CFLAGS = -g -O3 -msse -msse3 -ffast-math -Wall -I$(top_srcdir)/trunk/src/DttSP -Wno-unused-function @GTK_CFLAGS@
ghpsdr_CPPFLAGS = -I/usr/local/include \
         -I. -I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include\
         -I/usr/include/atk-1.0 -I/usr/include/cairo\
         -I/usr/include/pango-1.0 -I/usr/include/glib-2.0\
         -I/usr/lib/glib-2.0/include -I/usr/include/pixman-1\
         -I/usr/include/freetype2 -I/usr/include/libpng12 \
         -I/usr/include/gdk-pixbuf-2.0 \
         -I/usr/lib/x86_64-linux-gnu/glib-2.0/include \
         -I/usr/lib/i386-linux-gnu/glib-2.0/include/ \
         -I/usr/include


#### dspserver_CFLAGS = -g -O3 -Wall -I$(top_srcdir)/trunk/src/DttSP -Wno-unused-function -msse -msse3 -ffast-math -march=core2 
### OPTIONS=-g -O3 -msse -msse3 -ffast-math -Wall

#ghpsdr_LIBS=-L. -L../DttSP \
#     -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lpangoft2-1.0 -lgdk_pixbuf-2.0\
#     -lm -lpangocairo-1.0 -lgio-2.0 -lcairo -lpango-1.0 -lfreetype -lz\
#     -lfontconfig -lgobject-2.0 -lgmodule-2.0 -lglib-2.0

ghpsdr_LDADD = ../DttSP/libDttSP.a @GTK_LIBS@

#ghpsdr_LDADD = -L../DttSP -lDttSP
#-L. -L../DttSP -lDttSP -lpthread\
#     -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lpangoft2-1.0 -lgdk_pixbuf-2.0\
#     -lm -lpangocairo-1.0 -lgio-2.0 -lcairo -lpango-1.0 -lfreetype -lz\
#     -lfontconfig -lgobject-2.0 -lgmodule-2.0 -lglib-2.0 -lfftw3f -lportaudio


ghpsdr_SOURCES = \
agc.c          bandscope_control.c  display.c        iphone.c         meter_update.h  property.c   soundcard.h         transmit.c    xvtr_setup.h \
agc.h          bandscope_control.h  display.h        iphone.h         mode.c          property.h   spectrum_buffers.c  transmit.h \
audio.c        bandscope.h          display_setup.c  local_audio.c    mode.h          receiver.c   spectrum_buffers.h  version.h \
audio.h        bandscope_update.c   display_setup.h  local_audio.h    ozy.c           receiver.h   spectrum.c          vfo.c \
audiostream.c  bandscope_update.h   dttsp.h          main.c           ozy.h           setup.c      spectrum.h          vfo.h \
audiostream.h  bandstack.h          filter.c         main.h           port_audio.c    setup.h      spectrum_update.c   volume.c \
band.c         command.c            filter.h         meter.c         port_audio.h    sinewave.c   spectrum_update.h   volume.h \
band.h         command.h            frequency.c      meter.h         preamp.c        sinewave.h   subrx.c             xvtr.h \
bandscope.c    cw.h                 frequency.h      meter_update.c   preamp.h        soundcard.c  subrx.h             xvtr_setup.c 

