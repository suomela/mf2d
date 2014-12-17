#!/bin/bash

mkdir -p bin tmp || exit 1
cd src || exit 1
exec icpc -Wall -O3 -xHost -fopenmp -DNDEBUG mf2d.cc driver.cc filter.cc imageio.cc -lcfitsio -o ../bin/mf2d
