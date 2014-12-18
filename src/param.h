#ifndef PARAM_H
#define PARAM_H

const int MAX_H_1D = 1000;
const int MAX_H_2D = 100;

#define BLOCK_SIZES_1D 64,128,256,512,1024,2048,4096
#define BLOCK_SIZES_2D 16,32,64,128,256,512

// Reasonable values based on benchmarks

inline int choose_blocksize_1d(int h) {
    if (h < 10) {
        return 64;
    } else if (h < 20) {
        return 128;
    } else if (h < 40) {
        return 256;
    } else if (h < 80) {
        return 512;
    } else if (h < 160) {
        return 1024;
    } else if (h < 240) {
        return 2048;
    } else {
        return 4096;
    }
}

inline int choose_blocksize_2d(int h) {
    if (h < 3) {
        return 16;
    } else if (h < 5) {
        return 32;
    } else if (h < 13) {
        return 64;
    } else if (h < 34) {
        return 128;
    } else {
        return 256;
    }
}

#endif
