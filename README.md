2D Median Filter for Floating-Point Data
========================================

Filters 32-bit and 64-bit FITS images.

Alpha version, bugs are possible, use at your own risk.


Usage
-----

Usage:

    bin/mf2d radius input output-median output-difference

Example:

    bin/mf2d 15 example/test-1.fits a.fits b.fits

"Radius" is the window radius in pixels. For example, a radius
of 15 means a window of 31x31 pixels in total.

"Input" is a FITS file, 2 dimensions, floating point values.

"Output-median" will be a FITS file that contains the result
of median filtering.

"Output-difference" will be another FITS file that contains
the difference between "input" and "output-median".

All file names are given in a format supported by the cfitsio
library. In particular, prefix the output file names with an
exclamation mark if you want to overwrite existing files:

  bin/mf2d 15 example/test-1.fits '!a.fits' '!b.fits'


Performance
-----------

Test data:

  - example/test-1.fits
  - 1024x1024 pixels, 32-bit floats
  - radius = 0, 1, ..., 100

Detailed information available in directory "benchmark".


### Low-end laptop

1.7 GHz Intel Haswell, 2 cores, 4 threads.

Running time (block size B = 256):

  - r =  20:  0.24 s
  - r =  40:  0.38 s
  - r =  60:  0.54 s
  - r =  80:  0.69 s
  - r = 100:  1.05 s


### High-end server

2.6 GHz Intel Sandy Bridge, 2 x 8 cores, 32 threads.

Running time (block size B = 256):

  - r =  20:  0.07 s
  - r =  40:  0.12 s
  - r =  60:  0.14 s
  - r =  80:  0.17 s
  - r = 100:  0.27 s


Details
-------

Boundaries are handled by clipping the sliding window to
image boundaries. For example, while middle parts of the output
will be medians of (2r+1) x (2r+1) boxes, the corners of the
output will be medians of (r+1) x (r+1) boxes.

In the middle parts, the window always contains an odd number
of pixels, and hence the median is unique. Near the boundaries
we may have an even number of pixels in the window; in those
cases we will output the average of the two middle values.

The maximum radius is currently 100 (i.e., the maximum window
size is 201x201).


Compiling
---------

### OS X 10.10 and GCC 4.9

If needed, install gcc and cfitsio from Homebrew:

    brew install gcc cfitsio

Compile:

    compile/gcc-49-osx.sh

Test:

    test/test.sh


### OS X 10.10 and clang

(Not recommended: slow, will not use OpenMP.)

If needed, install cfitsio from Homebrew:

    brew install cfitsio

Compile:

    compile/clang-osx.sh

Test:

    test/test.sh


### Linux and GCC 4.7 or later

Make sure you have got GCC version 4.7 or later, e.g.:

    g++ --version
    g++-4.7 --version
    g++-4.8 --version
    g++-4.9 --version

If needed, install cfitsio. If installed in a non-standard
location, set the paths accordingly, e.g.:

    export CPATH=$HOME/opt/include
    export LIBRARY_PATH=$HOME/opt/lib

Depending on the name of your preferred C++ compiler, use
one of the following commands to compile:

    compile/gcc-linux.sh
    compile/gcc-47-linux.sh
    compile/gcc-48-linux.sh
    compile/gcc-49-linux.sh

Test:

    test/test.sh


### Linux and ICC 14.0 or later

As above, but use the following command to compile:

    compile/icc-linux.sh


Platforms and versions
----------------------

Tested on the following platforms:

  - OS X 10.10
  - Ubuntu 12.04
  - Ubuntu 14.04

With e.g. the following compilers:

  - GCC 4.7, 4.8, 4.9
  - ICC 14.0, 15.0
  - Apple LLVM version 6.0

Using the following libraries:

  - cfitsio 3.370


License
-------

Copyright (c) 2014, Jukka Suomela.

You can distribute and use this software under the MIT license:
http://opensource.org/licenses/MIT

To contact the author, see http://users.ics.aalto.fi/suomela/


Acknowledgements
----------------

Test data contributed by Jean-Eric Campagne.
