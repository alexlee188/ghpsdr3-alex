#!/bin/bash
source set_Qt-4.8-0_path.src 
./configure CFLAGS='-fopenmp -O3 -msse3' CXXFLAGS='-fopenmp -O3 -msse3'

