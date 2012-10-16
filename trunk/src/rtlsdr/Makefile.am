ACLOCAL_AMFLAGS = -I build-aux

bin_PROGRAMS = rtlsdr-server 
# rtlsdr-test

rtlsdr_server_CPPFLAGS	= -g -O3 -Wall @SIMD_FLAGS@


rtlsdr_server_SOURCES =			\
			listener.cpp	\
			receiver.cpp	\
			client.cpp	\
			server.cpp	\
			messages.h	\
			listener.h	\
			client.h	\
			receiver.h     

		
#   rtlsdr_test_CPPFLAGS = -g -D__TEST_MODULE__
#   rtlsdr_test_LDADD    = -lrt
#   rtlsdr_test_SOURCES  = \
#   			rtl-sdr.cpp	\
#   			rtl-sdr.h	\
#   			i2c.h           \
#   			tuner_e4000.cpp \
#   			tuner_e4000.h   \
#   			tuner_fc0013.cpp \
#   			tuner_fc0013.h  \
#   			tuner_fc0012.h  \
#   			tuner_fc0012.cpp
#
