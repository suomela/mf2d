#include <cstdlib>
#include <iostream>
#include "filter.h"

template <typename T>
static void check(int x, int y, int hx, int hy, const T* in, T* out, const T* expected) {
    median_filter<T>(x, y, hx, hy, 0, in, out);
    for (int i = 0; i < x*y; ++i) {
        if (out[i] != expected[i]) {
            std::cerr << "mismatch: " << i << ": " << out[i] << " vs. " << expected[i] << "\n";
            std::exit(EXIT_FAILURE);
        }
    }
}

template <typename T>
static void test() {
    const int x = 10;
    const int y = 5;
    T H = 0.5;
    T in[x * y] = {
        0,0,0,1,1,1,0,0,0,0,
        0,0,0,1,1,1,0,0,0,0,
        0,0,0,1,1,0,0,0,0,0,
        0,0,0,1,1,1,0,0,2,0,
        0,0,0,1,1,1,0,0,0,0
    };
    T zero[x * y] = {
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0
    };
    T exp01[x * y] = {
        0,0,0,1,1,1,0,0,0,0,
        0,0,0,1,1,1,0,0,0,0,
        0,0,0,1,1,1,0,0,0,0,
        0,0,0,1,1,1,0,0,0,0,
        0,0,0,1,1,1,0,0,1,0
    };
    T exp10[x * y] = {
        0,0,0,1,1,1,0,0,0,0,
        0,0,0,1,1,1,0,0,0,0,
        0,0,0,1,1,0,0,0,0,0,
        0,0,0,1,1,1,0,0,0,1,
        0,0,0,1,1,1,0,0,0,0
    };
    T exp20[x * y] = {
        0,0,0,1,1,1,0,0,0,0,
        0,0,0,1,1,1,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,1,1,1,1,0,0,0,
        0,0,0,1,1,1,0,0,0,0
    };
    T exp30[x * y] = {
        0,0,H,0,0,0,0,0,0,0,
        0,0,H,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,H,0,0,1,1,H,0,0,
        0,0,H,0,0,0,0,0,0,0
    };
    T exp40[x * y] = {
        0,H,0,0,0,0,0,0,0,0,
        0,H,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,H,0,0,0,0,H,1,H,0,
        0,H,0,0,0,0,0,0,0,0
    };
    T exp50[x * y] = {
        H,0,0,0,0,0,0,0,0,0,
        H,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        H,0,0,0,0,0,0,H,1,H,
        H,0,0,0,0,0,0,0,0,0
    };
    T exp11[x * y] = {
        0,0,0,1,1,1,0,0,0,0,
        0,0,0,1,1,1,0,0,0,0,
        0,0,0,1,1,1,0,0,0,0,
        0,0,0,1,1,1,0,0,0,0,
        0,0,0,1,1,1,0,0,0,0
    };
    T out[x * y];
    check(x, y, 0, 0, in, out, in);
    check(x, y, 0, 1, in, out, exp01);
    check(x, y, 0, 2, in, out, exp11);
    check(x, y, 0, 3, in, out, exp11);
    check(x, y, 0, 4, in, out, exp11);
    check(x, y, 0,99, in, out, exp11);
    check(x, y, 1, 0, in, out, exp10);
    check(x, y, 2, 0, in, out, exp20);
    check(x, y, 3, 0, in, out, exp30);
    check(x, y, 4, 0, in, out, exp40);
    check(x, y, 5, 0, in, out, exp50);
    check(x, y, 8, 0, in, out, zero);
    check(x, y,99, 0, in, out, zero);
    check(x, y, 1, 1, in, out, exp11);
    check(x, y, 2, 2, in, out, exp11);
    check(x, y, 4, 4, in, out, zero);
}

int main() {
    test<double>();
    test<float>();
    std::cout << "OK.\n";
}
