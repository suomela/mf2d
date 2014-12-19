#ifndef PARAM_H
#define PARAM_H

// Reasonable values based on benchmarks

inline int choose_blocksize_1d(int h) {
    return 8*(h + 2);
}

inline int choose_blocksize_2d(int h) {
    return 4*(h + 2);
}

#endif
