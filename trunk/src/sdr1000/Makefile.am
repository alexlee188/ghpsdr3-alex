bin_PROGRAMS = sdr1000
AUTOMAKE_OPTIONS = subdir-objects 

sdr1000_CFLAGS = -O3 -Wall -I./src/common -I./src/linux -I./src/linux/usb -I./server -Wno-unused-function $(THREAD_DEBUG_CFLAGS) @SIMD_FLAGS@
sdr1000_CPPFLAGS = -O3 -Wall -I./src/common -I./src/linux -I./src/linux/usb -I./server -Wno-unused-function $(THREAD_DEBUG_CFLAGS) @SIMD_FLAGS@

sdr1000_SOURCES = server/client.c \
 server/client.h \
 server/listener.c \
 server/listener.h \
 server/messages.h \
 server/receiver.c \
 server/receiver.h \
 server/sdr1000.c \
 server/sdr1000.h \
 server/sdr1000io.c \
 server/sdr1000io.h \
 server/server.cpp \
 server/server.h \
 server/util.c \
 server/util.h \
 src/common/ad9854.cpp \
 src/common/ad9854.h \
 src/common/ad9854_reg.cpp \
 src/common/ad9854_reg.h \
 src/common/common.h \
 src/common/hw_reg.cpp \
 src/common/hw_reg.h \
 src/common/pio_reg.cpp \
 src/common/pio_reg.h \
 src/common/rfe_reg.cpp \
 src/common/rfe_reg.h \
 src/linux/hw_sdr1000.cpp \
 src/linux/hw_sdr1000.h \
 src/linux/usb/sdr1kusb.cpp \
 src/linux/usb/sdr1kusb.h \
 src/linux/usb/usb.h
