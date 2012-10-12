bin_PROGRAMS = sdriq-test sdriq-server


sdriq_test_CPPFLAGS = -g -D__TEST_MODULE__  -DDEBUG=5 @SIMD_FLAGS@
sdriq_test_LDADD    = -lrt
sdriq_test_SOURCES  = sdriq.c sdriq.h


sdriq_server_CPPFLAGS	= -g -O3 -Wall  
sdriq_server_LDADD    = -lrt
sdriq_server_SOURCES =			\
          	sdriq.c	\
          	listener.c	\
          	receiver.c	\
          	client.c	\
          	server.c	\
          	messages.h	\
          	sdriq.h	\
          	listener.h	\
          	client.h	\
          	receiver.h     

        	
