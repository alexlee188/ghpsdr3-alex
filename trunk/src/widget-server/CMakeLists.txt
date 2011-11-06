project(widget-server)

set(CMAKE_VERBOSE_MAKEFILE OFF)

set (widget-server_SRCS
bandscope.c
listener.c
receiver.c
transmitter.c
util.c
client.c
ozy.c
ozyio.c
metis.c
server.c
operations.c
config_ops.c 


)

find_library (portaudio_LIB    portaudio)
find_library (pthread_LIB      pthread)
find_library (usb-1.0_LIB      usb-1.0)
find_library (usb_LIB          usb)
find_library (pulse-simple_LIB pulse-simple)

ADD_DEFINITIONS( -Wall -std=gnu99 -O3 )

set(LIBS ${LIBS} ${portaudio_LIBRARIES} ${pthread_LIBRARIES} ${usb_LIBRARIES}  ${m_LIBRARIES})

add_executable(widget-server ${widget-server_SRCS} )

target_link_libraries(widget-server ${portaudio_LIB} ${pthread_LIB} ${usb_LIB} ${usb-1.0_LIB}  ${pulse-simple_LIB})

INSTALL_TARGETS(/bin widget-server)
