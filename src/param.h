#ifndef PARAM_H
#define PARAM_H

const int MAX_H = 100;

#define BLOCK_SIZES 16,32,64,128,256,512

inline int choose_blocksize(int h) {
    // Reasonable values based on benchmarks
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
