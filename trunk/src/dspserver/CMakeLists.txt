project(dspserver)

set(CMAKE_VERBOSE_MAKEFILE ON)

set (dspserver_SRCS
     main.c
     ozy.c
     soundcard.c
     client.c
     audiostream.c
     register.c
)


find_library (portaudio_LIB    portaudio)
find_library (pthread_LIB      pthread)
find_library (usb-1.0_LIB      usb-1.0)
find_library (usb_LIB          usb)
find_library (pulse-simple_LIB pulse-simple)
find_library (config_LIB       config)
find_library (codec2_LIB       codec2)
find_library (fftw3f_LIB       fftw3f)

####

set (DttSP_DIR ${dspserver_BINARY_DIR}/../DttSP)

#find_library (libDttSP_LIB
#              NAMES DttSP
#              PATHS ${DttSP_DIR} ) 


ADD_DEFINITIONS ( -g -Wall -O3 -Wno-unused-function )

########## -I$(top_srcdir)/trunk/src/DttSP

include_directories(. ../DttSP )
link_directories(${DttSP_DIR})

add_executable(dspserver ${dspserver_SRCS} )

target_link_libraries(dspserver
    ${portaudio_LIB}
    ${pthread_LIB}
    ${usb_LIB}
    ${usb-1.0_LIB}
    ${pulse-simple_LIB}
    ${config_LIB}
    ${codec2_LIB}
    ${fftw3f_LIB}
    libDttSP.a
)


INSTALL_TARGETS(/bin dspserver)
