#!/bin/bash

# brew install cfitsio

mkdir -p bin tmp || exit 1
cd src || exit 1
exec clang++ -Wall -march=native -O3 -DNDEBUG mf2d.cc driver.cc filter.cc imageio.cc -lcfitsio -o ../bin/mf2d
