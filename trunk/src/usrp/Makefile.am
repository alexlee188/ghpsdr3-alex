ACLOCAL_AMFLAGS = -I build-aux
bin_PROGRAMS				= usrp_server
CC					= g++

usrp_server_CPPFLAGS	= -g -O3 -Wall @SIMD_FLAGS@
usrp_server_LDADD = -lgomp 

usrp_server_SOURCES = \
		bandscope.c  \
		listener.c \
		receiver.c \
		transmitter.c \
		util.c \
		client.c \
		usrp.c \
		server.c \
		usrp_audio.c \
		resampler.c \
		bandscope.h \
		buffer.h \
		client.h \
		listener.h \
		messages.h \
		receiver.h  transmitter.h  usrp_audio.h  usrp.h  util.h resampler.h

