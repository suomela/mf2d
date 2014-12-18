#include <cmath>
#include <iostream>
#include <stdexcept>
#include "driver.h"
#include "filter.h"
#include "imageio.h"
#include "param.h"
#include "timer.h"

//// Parameters for 1D and 2D filtering

struct Param {
    const int* block_sizes;
    int max_h;
    int benchmark_step;
};

const int block_sizes_1d[] = {BLOCK_SIZES_1D, 0};
const int block_sizes_2d[] = {BLOCK_SIZES_2D, 0};
const Param param1d = {block_sizes_1d, MAX_H_1D, 10};
const Param param2d = {block_sizes_2d, MAX_H_2D, 1};

template <typename T>
static const Param* get_param(const Driver<T,Image1D<T> > *tag) {
    return &param1d;
}

template <typename T>
static const Param* get_param(const Driver<T,Image2D<T> > *tag) {
    return &param2d;
}

//// Helper functions

template <typename T>
static void filter(int h, int blockhint, Image1D<T> in, Image1D<T> out) {
    median_filter_1d<T>(in.x, h, blockhint, in.p, out.p);
}

template <typename T>
static void filter(int h, int blockhint, Image2D<T> in, Image2D<T> out) {
    median_filter_2d<T>(in.x, in.y, h, h, blockhint, in.p, out.p);
}

template <typename T>
static void compare(T** prev, const T* cur, int size) {
    if (*prev) {
        for (int i = 0; i < size; ++i) {
            T a = cur[i];
            T b = (*prev)[i];
            bool ok = (a == b) || (std::isnan(a) && std::isnan(b));
            if (!ok) {
                throw std::runtime_error("output mismatch");
            }
        }
    } else {
        *prev = new T[size];
        for (int i = 0; i < size; ++i) {
            (*prev)[i] = cur[i];
        }
    }
}

//// Method implementations

template <typename T, typename I>
int Driver<T,I>::max_h() {
    return get_param(this)->max_h;
}

template <typename T, typename I>
void Driver<T,I>::process(int h) {
    filter<T>(h, 0, in, out);
}

template <typename T, typename I>
void Driver<T,I>::diff() {
    for (int i = 0; i < out.size(); ++i) {
        out.p[i] = in.p[i] - out.p[i];
    }
}

template <typename T, typename I>
void Driver<T,I>::write(const char* filename) {
    write_image(filename, out);
}

template <typename T, typename I>
void Driver<T,I>::benchmark() {
    const Param* param = get_param(this);
    for (int h = 0; h <= param->max_h; h += param->benchmark_step) {
        std::cout << h << std::flush;
        T* prev = 0;
        for (const int* block = param->block_sizes; *block; ++block) {
            if (*block < 2.5*h+1) {
                std::cout << "\t-" << std::flush;
            } else {
                Timer timer;
                filter<T>(h, *block, in, out);
                double t = timer.peek();
                std::cout << "\t" << t << std::flush;
                compare<T>(&prev, out.p, out.size());
            }
        }
        if (prev) {
            delete[] prev;
        }
        std::cout << std::endl;
    }
}

//// Versions

template class Driver<float, Image1D<float> >;
template class Driver<float, Image2D<float> >;
template class Driver<double, Image1D<double> >;
template class Driver<double, Image2D<double> >;
