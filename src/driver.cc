#include <cmath>
#include <iostream>
#include <stdexcept>
#include "driver.h"
#include "filter.h"
#include "imageio.h"
#include "param.h"
#include "timer.h"

template <typename T>
void Driver2D<T>::process() {
    median_filter_2d<T>(in.x, in.y, settings.h, settings.h, 0, in.p, out.p);
    write_image(settings.target_med, out);
    for (int i = 0; i < out.size(); ++i) {
        out.p[i] = in.p[i] - out.p[i];
    }
    write_image(settings.target_diff, out);
}

template <typename T>
void Driver1D<T>::process() {
    median_filter_1d<T>(in.x, settings.h, 0, in.p, out.p);
    write_image(settings.target_med, out);
    for (int i = 0; i < out.size(); ++i) {
        out.p[i] = in.p[i] - out.p[i];
    }
    write_image(settings.target_diff, out);
}

template <typename T>
void Driver2D<T>::benchmark() {
    const int block_sizes[] = {BLOCK_SIZES_2D};
    const int n_block_sizes = sizeof(block_sizes)/sizeof(int);
    for (int h = 0; h <= MAX_H_2D; ++h) {
        std::cout << h << std::flush;
        T* prev = 0;
        for (int iblock = 0; iblock < n_block_sizes; ++iblock) {
            int block = block_sizes[iblock];
            if (block < 2.5*h+1) {
                std::cout << "\t-" << std::flush;
            } else {
                Timer timer;
                median_filter_2d<T>(in.x, in.y, h, h, block, in.p, out.p);
                double t = timer.peek();
                std::cout << "\t" << t << std::flush;
                if (prev) {
                    for (int i = 0; i < out.size(); ++i) {
                        T a = out.p[i];
                        T b = prev[i];
                        bool ok = (a == b) || (std::isnan(a) && std::isnan(b));
                        if (!ok) {
                            throw std::runtime_error("output mismatch");
                        }
                    }
                } else {
                    prev = new T[out.size()];
                    for (int i = 0; i < out.size(); ++i) {
                        prev[i] = out.p[i];
                    }
                }
            }
        }
        if (prev) {
            delete[] prev;
        }
        std::cout << std::endl;
    }
}

template <typename T>
void Driver1D<T>::benchmark() {
    const int block_sizes[] = {BLOCK_SIZES_1D};
    const int n_block_sizes = sizeof(block_sizes)/sizeof(int);
    for (int h = 0; h <= MAX_H_1D; ++h) {
        std::cout << h << std::flush;
        T* prev = 0;
        for (int iblock = 0; iblock < n_block_sizes; ++iblock) {
            int block = block_sizes[iblock];
            if (block < 2.5*h+1) {
                std::cout << "\t-" << std::flush;
            } else {
                Timer timer;
                median_filter_1d<T>(in.x, h, block, in.p, out.p);
                double t = timer.peek();
                std::cout << "\t" << t << std::flush;
                if (prev) {
                    for (int i = 0; i < out.size(); ++i) {
                        T a = out.p[i];
                        T b = prev[i];
                        bool ok = (a == b) || (std::isnan(a) && std::isnan(b));
                        if (!ok) {
                            throw std::runtime_error("output mismatch");
                        }
                    }
                } else {
                    prev = new T[out.size()];
                    for (int i = 0; i < out.size(); ++i) {
                        prev[i] = out.p[i];
                    }
                }
            }
        }
        if (prev) {
            delete[] prev;
        }
        std::cout << std::endl;
    }
}

template class Driver1D<float>;
template class Driver1D<double>;
template class Driver2D<float>;
template class Driver2D<double>;
