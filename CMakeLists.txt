project(ghpsdr3)
cmake_minimum_required(VERSION 2.8.0)

# Versions
set(ghpsdr3_VERSION_MAJOR 1)
set(ghpsdr3_VERSION_MINOR 6)
set(ghpsdr3_VERSION_PATCH 0)
set(ghpsdr3_VERSION ${ghpsdr3_VERSION_MAJOR}.${ghpsdr3_VERSION_MINOR}.${ghpsdr3_VERSION_PATCH})



# Source
add_subdirectory(trunk/src/QtRadio)
