bin_PROGRAMS = hiqsdr-test hiqsdr-server

hiqsdr_test_CPPFLAGS = -g -D__TEST_MODULE__
hiqsdr_test_LDADD    = -lrt
hiqsdr_test_SOURCES  = hiqsdr.cpp hiqsdr.h

hiqsdr_server_CPPFLAGS	= -g -O3 -Wall @SIMD_FLAGS@


hiqsdr_server_SOURCES =			\
			hiqsdr.cpp	\
			listener.cpp	\
			receiver.cpp	\
			client.cpp	\
			server.cpp	\
			messages.h	\
			hiqsdr.h	\
			listener.h	\
			client.h	\
			receiver.h     

		
