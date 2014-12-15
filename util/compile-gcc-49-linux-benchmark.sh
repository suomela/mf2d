#!/bin/bash

mkdir -p bin tmp || exit 1
cd src || exit 1
exec g++ -Wall -Wextra -Wold-style-cast -Woverloaded-virtual -Wsign-promo -Wdouble-promotion -Wconversion -Wsign-conversion -Wmissing-declarations -Wredundant-decls -Wno-unused-parameter -Wno-float-equal -Wno-array-bounds -march=native -O3 -fopenmp -std=c++11 -DNDEBUG mf2d-benchmark.cc driver.cc filter.cc imageio.cc -lcfitsio -o ../bin/mf2d-benchmark
