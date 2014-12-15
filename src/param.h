#ifndef PARAM_H
#define PARAM_H

constexpr int MAX_H {100};

#define BLOCK_SIZES 16,32,64,128,256,512

inline int choose_blocksize(int h) {
    // Reasonable values based on benchmarks
    if (h < 3) {
        return 16;
    } else if (h < 5) {
        return 32;
    } else if (h < 12) {
        return 64;
    } else if (h < 25) {
        return 128;
    } else if (h < 50) {
        return 256;
    } else {
        return 512;
    }
}

#endif
