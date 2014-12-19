import os
import sys

def build(dir, **d):
    env = Environment(**d)
    SConscript('src/SConscript', variant_dir=dir, exports='env')

gccdiag='-fdiagnostics-color=auto -Werror -Wall -Wextra -Wold-style-cast -Woverloaded-virtual -Wsign-promo -Wdouble-promotion -Wconversion -Wsign-conversion -Wmissing-declarations -Wredundant-decls -Wno-unused-parameter -Wno-float-equal -Wno-array-bounds'
clangdiag='-Weverything -Werror -Wno-padded -Wno-weak-template-vtables -Wno-weak-vtables -Wno-source-uses-openmp -Wno-unused-parameter -Wno-float-equal'
gccopt='-march=native -O3 -Wa,-q'
clangopt='-march=native -O3'
gcclink='-fopenmp'
gcccommon=gccdiag + ' ' + gccopt + ' -fopenmp'
clangcommon=clangdiag + ' ' + clangopt
gccsanity='-fsanitize=address -fsanitize=undefined'
build('build',
    CXX='g++-4.9',
    CXXFLAGS=gcccommon,
    CPPDEFINES=['NDEBUG'],
    LINKFLAGS=gcclink,
    LIBS='cfitsio',
)
build('build-debug',
    CXX='g++-4.9',
    CXXFLAGS=gcccommon + ' -g ' + gccsanity,
    LINKFLAGS=gcclink + ' ' + gccsanity,
    LIBS='cfitsio',
)
build('build-naive',
    CXX='g++-4.9',
    CXXFLAGS=gcccommon,
    CPPDEFINES=['NAIVE'],
    LINKFLAGS=gcclink,
    LIBS='cfitsio',
)
build('build-clang',
    CXX='clang++',
    CXXFLAGS=clangcommon,
    LIBS='cfitsio',
)
