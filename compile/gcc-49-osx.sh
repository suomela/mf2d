#!/bin/bash

# brew install gcc cfitsio

# If all else fails, try to remove both "-Wa,-q" and "-march=native".

mkdir -p bin tmp || exit 1
cd src || exit 1
exec g++-4.9 -Wall -Wno-array-bounds -march=native -O3 -Wa,-q -fopenmp -DNDEBUG mf2d.cc driver.cc filter.cc imageio.cc -lcfitsio -o ../bin/mf2d
