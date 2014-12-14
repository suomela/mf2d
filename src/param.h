#ifndef PARAM_H
#define PARAM_H

constexpr int MAX_H {50};

#define BLOCK_SIZES 64,128,256,512

inline int choose_blocksize(int h) {
    // Reasonable values based on benchmarks
    if (h < 12) {
        return 64;
    } else if (h < 25) {
        return 128;
    } else {
        return 256;
    }
}

#endif
