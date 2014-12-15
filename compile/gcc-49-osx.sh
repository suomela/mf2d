#!/bin/bash

# brew install gcc cfitsio

mkdir -p bin tmp || exit 1
cd src || exit 1
exec g++-4.9 -fdiagnostics-color=auto -Wall -Wextra -Wold-style-cast -Woverloaded-virtual -Wsign-promo -Wdouble-promotion -Wconversion -Wsign-conversion -Wmissing-declarations -Wredundant-decls -Wno-unused-parameter -Wno-float-equal -Wno-array-bounds -march=native -O3 -Wa,-q -fopenmp -std=c++11 -DNDEBUG -lcfitsio mf2d.cc driver.cc filter.cc imageio.cc -o ../bin/mf2d
