project(ghpsdr3)
cmake_minimum_required(VERSION 2.8.0)

# Versions
set(ghpsdr3_VERSION_MAJOR 1)
set(ghpsdr3_VERSION_MINOR 6)
set(ghpsdr3_VERSION_PATCH 0)
set(ghpsdr3_VERSION ${ghpsdr3_VERSION_MAJOR}.${ghpsdr3_VERSION_MINOR}.${ghpsdr3_VERSION_PATCH})



# Source
add_subdirectory(trunk/src/DttSP)
add_subdirectory(trunk/src/dspserver)
add_subdirectory(trunk/src/widget-server)
add_subdirectory(trunk/src/softrock)
add_subdirectory(trunk/src/QtRadio)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
