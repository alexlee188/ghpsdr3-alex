#!/bin/bash
source set_Qt-4.8-0_path.src 
./configure CC=clang CXX=clang CFLAGS='-O3 -msse3' CXXFLAGS='-O3 -sse3'

