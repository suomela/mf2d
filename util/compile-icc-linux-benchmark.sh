#!/bin/bash

mkdir -p bin tmp || exit 1
cd src || exit 1
exec icpc -Wall -O3 -xHost -fopenmp -std=c++11 -DNDEBUG mf2d-benchmark.cc driver.cc filter.cc imageio.cc -lcfitsio -o ../bin/mf2d-benchmark
