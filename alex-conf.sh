#!/bin/bash
./configure CFLAGS='-fopenmp -O3 -march=native' CXXFLAGS='-fopenmp -flto -O3 -march=native'

