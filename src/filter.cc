#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <x86intrin.h>
#include "param.h"

// Bit manipulation

constexpr int MASK64 { 64-1 };
constexpr int SHIFT64 { 6 };
constexpr uint64_t ONE64 { 1 };


// Find nth bit that is set and return its index
// (64 = no such bit)

inline int findnth64(uint64_t x, int n) {
#ifdef __AVX2__
    uint64_t bit {_pdep_u64(ONE64 << n, x)};
    return static_cast<int>(__tzcnt_u64(bit));
#else
    int j {0};
    int i {0};
    while (x) {
        j += static_cast<int>(x & 1);
        if (j > n) {
            return i;
        }
        x >>= 1;
        ++i;
    }
    return 64;
#endif
}


// Data structure for the sliding within a B*B block.

template <int B>
struct Window {
    constexpr static int BB {B * B};
    constexpr static int BB64 {BB / 64};

    // A vector with B*B bits that keeps track of the contents of the
    // sliding window. The elements of the block are sorted and
    // numbered with integers [0,B*B). Bit number s is on iff element
    // s is inside the window.
    uint64_t buf[BB64];
    // count[i] = popcount(buf[i])
    int count[BB64];
    // The current guess is that the median is in buf[p].
    // The guess is corrected by calling "fix".
    int p;
    // small = count[0] + ... + count[p-1]
    int small;
    // large = count[p] + ... + count[BB64-1]
    int large;

    inline void clear()
    {
        for (int i {0}; i < BB64; ++i) {
            buf[i] = 0;
            count[i] = 0;
        }
        p = BB64 / 2;
        small = 0;
        large = 0;
    }

    inline void add(int s) {
        int i { s >> SHIFT64 };
        int j { s & MASK64 };
        assert(!(buf[i] & (ONE64 << j)));
        buf[i] |= (ONE64 << j);
        ++count[i];
        if (i < p) {
            ++small;
        } else {
            ++large;
        }
    }

    inline void del(int s) {
        int i { s >> SHIFT64 };
        int j { s & MASK64 };
        assert(buf[i] & (ONE64 << j));
        buf[i] &= ~(ONE64 << j);
        --count[i];
        if (i < p) {
            --small;
        } else {
            --large;
        }
    }

    inline void fix() {
        while (small >= large) {
            --p;
            small -= count[p];
            large += count[p];
        }
        while (small + count[p] < large - count[p]) {
            small += count[p];
            large -= count[p];
            ++p;
        }
    }

    inline bool even() const {
        return ((small + large) & 1) == 0;
    }

    // Rounds down if even number of points
    inline int med() const {
        assert(small < large);
        int n {(large - small - 1) / 2};
        assert(n < count[p]);
        int j {findnth64(buf[p], n)};
        assert(j < 64);
        return (p << SHIFT64) | j;
    }
};


// Grid dimensions.

template <int B>
struct Dim {
    Dim(int size_, int h_)
        : size{size_},
          h{h_},
          step{calc_step(h_)},
          count{calc_count(size_, h_)}
    {
        assert(2 * h + 1 < B);
        assert(count >= 1);
        assert(2 * h + count * step >= size);
        assert(2 * h + (count - 1) * step < size || count == 1);
    }

    const int size;
    const int h;
    const int step;
    const int count;

private:
    inline static int calc_step(int h) {
        return B - 2*h;
    }

    inline static int calc_count(int size, int h) {
        if (size <= B) {
            return 1;
        } else {
            int interior {size - 2 * h};
            int step {calc_step(h)};
            return (interior + step - 1) / step;
        }
    }
};


// Slot i in the grid.

template <int B>
struct BDim {
    BDim(Dim<B> dim_) : dim{dim_} {
        set(0);
    }

    inline void set(int i) {
        bool is_first {i == 0};
        bool is_last {i + 1 == dim.count};
        start = dim.step * i;
        int end;
        if (is_last) {
            end = dim.size;
        } else {
            end = 2 * dim.h + (i + 1) * dim.step;
        }
        size = end - start;
        b0 = is_first ? 0 : dim.h;
        b1 = is_last ? size : size - dim.h;
    }

    // The window around point v is [w0(v), w1(v)).
    // 0 <= w0(v) <= v < w1(v) <= size
    inline int w0(int v) {
        assert(b0 <= v);
        assert(v < b1);
        return std::max(0, v - dim.h);
    }

    inline int w1(int v) {
        assert(b0 <= v);
        assert(v < b1);
        return std::min(v + 1 + dim.h, size);
    }

    // Block i is located at coordinates [start, end) in the image.
    // Within the block, median is needed for coordinates [b0, b1).
    // 0 <= start < end < dim.size
    // 0 <= b0 < b1 < size <= B
    const Dim<B> dim;
    int start;
    int size;
    int b0;
    int b1;
};


// MedCalc.run(i,j) calculates medians for block (i,j).

template <typename T, int B>
class MedCalc {
public:
    MedCalc(Dim<B> dimx_, Dim<B> dimy_, const T* in_, T* out_)
        : bx{dimx_}, by{dimy_}, in{in_}, out{out_}
    {}

    void run(int bx_, int by_)
    {
        bx.set(bx_);
        by.set(by_);
        calc_rank();
#ifdef NAIVE
        medians_naive();
#else
        medians();
#endif
    }

private:
    void calc_rank() {
        for (int y {0}; y < by.size; ++y) {
            for (int x {0}; x < bx.size; ++x) {
                sorted[y * bx.size + x] = std::make_pair(get_pixel(x, y), y * B + x);
            }
        }
        std::sort(sorted, sorted + by.size * bx.size);
        for (int i {0}; i < by.size * bx.size; ++i) {
            rank[sorted[i].second] = i;
        }
    }

    // Simple baseline implementation for testing
    void medians_naive() {
        for (int y {by.b0}; y < by.b1; ++y) {
            for (int x {bx.b0}; x < bx.b1; ++x) {
                window.clear();
                add_block(bx.w0(x), bx.w1(x), by.w0(y), by.w1(y));
                set_med(x, y);
            }
        }
    }

    void medians() {
        window.clear();
        int x {bx.b0};
        int y {by.b0};
        add_block(bx.w0(x), bx.w1(x), by.w0(y), by.w1(y));
        set_med(x, y);
        bool down {true};
        while (true) {
            bool right {false};
            if (down) {
                if (y + 1 == by.b1) {
                    right = true;
                    down = false;
                }
            } else {
                if (y == by.b0) {
                    right = true;
                    down = true;
                }
            }
            if (right) {
                if (x + 1 == bx.b1) {
                    break;
                }
            }
            if (right) {
                del_block(bx.w0(x), bx.w0(x+1), by.w0(y), by.w1(y));
                ++x;
                add_block(bx.w1(x-1), bx.w1(x), by.w0(y), by.w1(y));
            } else if (down) {
                del_block(bx.w0(x), bx.w1(x), by.w0(y), by.w0(y+1));
                ++y;
                add_block(bx.w0(x), bx.w1(x), by.w1(y-1), by.w1(y));
            } else {
                del_block(bx.w0(x), bx.w1(x), by.w1(y-1), by.w1(y));
                --y;
                add_block(bx.w0(x), bx.w1(x), by.w0(y), by.w0(y+1));
            }
            set_med(x, y);
        }
    }

    inline void add_block(int x0, int x1, int y0, int y1) {
        for (int y {y0}; y < y1; ++y) {
            for (int x {x0}; x < x1; ++x) {
                add(x, y);
            }
        }
    }

    inline void del_block(int x0, int x1, int y0, int y1) {
        for (int y {y0}; y < y1; ++y) {
            for (int x {x0}; x < x1; ++x) {
                del(x, y);
            }
        }
    }

    inline void add(int x, int y) {
        window.add(rank[y * B + x]);
    }

    inline void del(int x, int y) {
        window.del(rank[y * B + x]);
    }

    inline void set_med(int x, int y) {
        window.fix();
        int med1 {window.med()};
        T value {sorted[med1].first};
        if (window.even()) {
            window.del(med1);
            window.fix();
            assert(!window.even());
            int med2 {window.med()};
            window.add(med1);
            assert(med2 > med1);
            value += sorted[med2].first;
            value /= 2;
        }
        set_pixel(x, y, value);
    }

    inline T get_pixel(int x, int y) const {
        int imgy {y + by.start};
        int imgx {x + bx.start};
        return in[imgy * bx.dim.size + imgx];
    }

    inline void set_pixel(int x, int y, T value) {
        int imgy {y + by.start};
        int imgx {x + bx.start};
        out[imgy * bx.dim.size + imgx] = value;
    }

    constexpr static int BB {Window<B>::BB};
    std::pair<T, int> sorted[BB];
    int rank[BB];
    Window<B> window;
    BDim<B> bx;
    BDim<B> by;
    const T* const in;
    T* const out;
};


template <typename T, int B>
void median_filter_impl(int x, int y, int hx, int hy, const T* in, T* out) {
    if (2 * hx + 1 > B || 2 * hy + 1 > B) {
        throw std::invalid_argument("window too large for this block size");
    }
    Dim<B> dimx(x, hx);
    Dim<B> dimy(y, hy);
    #pragma omp parallel
    {
        MedCalc<T,B>* mc = new MedCalc<T,B>(dimx, dimy, in, out);
        #pragma omp for collapse(2)
        for (int by = 0; by < dimy.count; ++by) {
            for (int bx = 0; bx < dimx.count; ++bx) {
                mc->run(bx, by);
            }
        }
        delete[] mc;
    }
}


template <typename T>
void median_filter(int x, int y, int hx, int hy, int blockhint, const T* in, T* out) {
    int h {std::max(hx, hy)};
    if (h > MAX_H) {
        throw std::invalid_argument("window too large");
    }
    int blocksize {blockhint ? blockhint : choose_blocksize(h)};
    switch (blocksize) {
    case 64:
        median_filter_impl<T,64>(x, y, hx, hy, in, out);
        break;
    case 128:
        median_filter_impl<T,128>(x, y, hx, hy, in, out);
        break;
    case 256:
        median_filter_impl<T,256>(x, y, hx, hy, in, out);
        break;
    case 512:
        median_filter_impl<T,512>(x, y, hx, hy, in, out);
        break;
    default:
        throw std::invalid_argument("unsupported block size");
    }
}

template void median_filter<float>(int x, int y, int hx, int hy, int blockhint, const float* in, float* out);
template void median_filter<double>(int x, int y, int hx, int hy, int blockhint, const double* in, double* out);

template <typename T>
void median_filter(int x, int y, int hx, int hy, const T* in, T* out);

