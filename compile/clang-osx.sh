#!/bin/bash

# brew install cfitsio

mkdir -p bin tmp || exit 1
cd src || exit 1
exec clang++ -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-weak-template-vtables -Wno-weak-vtables -Wno-source-uses-openmp -Wno-unused-parameter -Wno-float-equal -march=native -O3 -std=c++11 -DNDEBUG -lcfitsio mf2d.cc driver.cc filter.cc imageio.cc -o ../bin/mf2d
