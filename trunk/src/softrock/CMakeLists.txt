project(softrock)

set(CMAKE_VERBOSE_MAKEFILE OFF)

set (softrock_SRCS
     client.c
     config_ops.c
     jackio.c
     listener.c
     operations.c
     receiver.c
     server.c
     softrock.c
     softrockio.c
     util.c
)


find_library(portaudio_LIB    portaudio)
find_library(pthread_LIB      pthread)
find_library(usbLIB           usb)
find_library(pulse-simple_LIB pulse-simple)

ADD_DEFINITIONS(   -Wall -std=gnu99 -g -O3 -Wall)

set(LIBS ${LIBS} ${portaudio_LIBRARIES} ${pthread_LIBRARIES} ${usb_LIBRARIES}  ${m_LIBRARIES})

add_executable(softrock ${softrock_SRCS} )

target_link_libraries(softrock ${portaudio_LIB} ${pthread_LIB} ${usbLIB} ${pulse-simple_LIB})

INSTALL_TARGETS(/bin softrock)