#!/bin/bash
source set_Qt-4.8-0_path.src 
./configure CFLAGS='-fopenmp -O3 -mavx' CXXFLAGS='-fopenmp -O3 -mavx'

