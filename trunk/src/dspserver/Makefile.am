

bin_PROGRAMS = dspserver

noinst_LIBRARIES = libsdrutil.a

dspserver_CFLAGS = -O3 -Wall -I$(top_srcdir)/trunk/src/DttSP -Wno-unused-function $(THREAD_DEBUG_CFLAGS) @SIMD_FLAGS@
dspserver_LDADD = ../DttSP/libDttSP.a libsdrutil.a -lrt -lssl -lcrypto 

dspserver_SOURCES = main.c\
         ozy.c\
         soundcard.c\
         client.c\
         audiostream.c \
	 audiostream.h  \
buffer.h       \
client.h       \
codec2loc.h    \
dttsp.h        \
main.h         \
ozy.h          \
soundcard.h    \
version.h	   \
register.h	   \
register.c     \
rtp.h          \
rtp.c          \
G711A.h        \
G711A.c

libsdrutil_a_SOURCES = util.h         \
                       util.c

