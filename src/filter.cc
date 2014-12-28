#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <stdint.h>
#include <x86intrin.h>

const uint64_t ONE64 = 1;


// Reasonable values based on benchmarks

inline int choose_blocksize_1d(int h) {
    return 8*(h + 2);
}

inline int choose_blocksize_2d(int h) {
    return 4*(h + 2);
}



// Find nth bit that is set and return its index
// (no such bit: output undefined)

inline int findnth64(uint64_t x, int n) {
#ifdef __AVX2__
    x = _pdep_u64(ONE64 << n, x);
#else
    for (int i = 0; i < n; ++i) {
        x &= x - 1;
    }
#endif
    return __builtin_ctzll(x);
}

inline int popcnt64(uint64_t x) {
    return __builtin_popcountll(x);
}


// Data structure for the sliding window.

class Window {
public:
    Window(int bb)
        : words(get_words(bb)),
          buf(new uint64_t[words])
    {}

    ~Window() {
        delete[] buf;
    }

    inline void clear()
    {
        for (int i = 0; i < words; ++i) {
            buf[i] = 0;
        }
        half[0] = 0;
        half[1] = 0;
        p = words / 2;
    }

    inline void update(int op, int s) {
        assert(op == -1 || op == +1);
        int i = s >> SHIFT64;
        int j = s & MASK64;
        if (op == +1) {
            assert(!(buf[i] & (ONE64 << j)));
        } else {
            assert(buf[i] & (ONE64 << j));
        }
        buf[i] ^= (ONE64 << j);
        half[i >= p] += op;
    }

    inline int size() const {
        return half[0] + half[1];
    }

    inline bool empty() const {
        return size() == 0;
    }

    inline bool even() const {
        return (size() & 1) == 0;
    }

    // Rounds down if even number of points
    inline int med() {
        while (half[0] >= half[1]) {
            --p;
            half[0] -= popcnt64(buf[p]);
            half[1] += popcnt64(buf[p]);
        }
        while (half[0] + popcnt64(buf[p]) < half[1] - popcnt64(buf[p])) {
            half[0] += popcnt64(buf[p]);
            half[1] -= popcnt64(buf[p]);
            ++p;
        }
        assert(half[0] < half[1]);
        int n = (half[1] - half[0] - 1) / 2;
        assert(n < popcnt64(buf[p]));
        int j = findnth64(buf[p], n);
        return (p << SHIFT64) | j;
    }

private:
    static inline int get_words(int bb) {
        assert(bb >= 1);
        return (bb + WORDSIZE - 1) / WORDSIZE;
    }

    static const int WORDSIZE = 64;
    static const int MASK64 = 64-1;
    static const int SHIFT64 = 6;

    // Size of buf.
    int words;
    // Bit number s is on iff element s is inside the window.
    uint64_t *buf;
    // half[0] = popcount of buf[0] ... buf[p-1]
    // half[1] = popcount of buf[p] ... buf[words-1]
    int half[2];
    // The current guess is that the median is in buf[p].
    int p;
};


// Grid dimensions.

class Dim {
public:
    Dim(int b_, int size_, int h_)
        : size(size_),
          h(h_),
          step(calc_step(b_, h_)),
          count(calc_count(b_, size_, h_))
    {
        assert(2 * h + 1 < b_);
        assert(count >= 1);
        assert(2 * h + count * step >= size);
        assert(2 * h + (count - 1) * step < size || count == 1);
    }

    const int size;
    const int h;
    const int step;
    const int count;

private:
    inline static int calc_step(int b, int h) {
        return b - 2*h;
    }

    inline static int calc_count(int b, int size, int h) {
        if (size <= b) {
            return 1;
        } else {
            int interior = size - 2 * h;
            int step = calc_step(b, h);
            return (interior + step - 1) / step;
        }
    }
};


// Slot i in the grid.

struct BDim {
    BDim(Dim dim_) : dim(dim_) {
        set(0);
    }

    inline void set(int i) {
        bool is_first = (i == 0);
        bool is_last = (i + 1 == dim.count);
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
    inline int w0(int v) const {
        assert(b0 <= v);
        assert(v < b1);
        return std::max(0, v - dim.h);
    }

    inline int w1(int v) const {
        assert(b0 <= v);
        assert(v < b1);
        return std::min(v + 1 + dim.h, size);
    }

    // Block i is located at coordinates [start, end) in the image.
    // Within the block, median is needed for coordinates [b0, b1).
    // 0 <= start < end < dim.size
    // 0 <= b0 < b1 < size <= dim.b
    const Dim dim;
    int start;
    int size;
    int b0;
    int b1;
};


template <typename T, typename R>
class WindowRank {
public:
    WindowRank(int bb_)
        : sorted(new std::pair<T,R>[bb_]),
          rank(new R[bb_]),
          window(bb_),
          bb(bb_)
    {
        assert(static_cast<unsigned>(bb_) < NAN_MARKER);
    }

    ~WindowRank()
    {
        delete[] sorted;
        delete[] rank;
    }

    void init_start() {
        size = 0;
    }

    inline void init_feed(T value, R slot) {
        if (std::isnan(value)) {
            rank[slot] = NAN_MARKER;
        } else {
            sorted[size] = std::make_pair(value, slot);
            ++size;
        }
    }

    void init_finish() {
        std::sort(sorted, sorted + size);
        for (int i = 0; i < size; ++i) {
            rank[sorted[i].second] = static_cast<R>(i);
        }
    }

    inline void clear() {
        window.clear();
    }

    inline void update(int op, R slot) {
        R s = rank[slot];
        if (s != NAN_MARKER) {
            window.update(op, static_cast<int>(s));
        }
    }

    inline T get_med() {
        if (window.empty()) {
            return std::numeric_limits<T>::quiet_NaN();
        } else {
            int med1 = window.med();
            T value = sorted[med1].first;
            if (window.even()) {
                window.update(-1, med1);
                assert(!window.even());
                int med2 = window.med();
                window.update(+1, med1);
                assert(med2 > med1);
                value += sorted[med2].first;
                value /= 2;
            }
            return value;
        }
    }

private:
    std::pair<T,R>* sorted;
    R* rank;
    Window window;
    int bb;
    int size;
    static const R NAN_MARKER = static_cast<R>(-1);
};


// MedCalc2D.run(i,j) calculates medians for block (i,j).

template <typename T, typename R>
class MedCalc2D {
public:
    MedCalc2D(int b_, Dim dimx_, Dim dimy_, const T* in_, T* out_)
        : wr(b_ * b_), bx(dimx_), by(dimy_), in(in_), out(out_)
    {}

    void run(int bx_, int by_)
    {
        bx.set(bx_);
        by.set(by_);
        calc_rank();
        medians();
    }

private:
    void calc_rank() {
        wr.init_start();
        for (int y = 0; y < by.size; ++y) {
            for (int x = 0; x < bx.size; ++x) {
                wr.init_feed(in[coord(x, y)], pack(x, y));
            }
        }
        wr.init_finish();
    }

    void medians() {
#ifdef NAIVE
        for (int y = by.b0; y < by.b1; ++y) {
            for (int x = bx.b0; x < bx.b1; ++x) {
                wr.clear();
                update_block(+1, bx.w0(x), bx.w1(x), by.w0(y), by.w1(y));
                set_med(x, y);
            }
        }
#else
        wr.clear();
        int x = bx.b0;
        int y = by.b0;
        update_block(+1, bx.w0(x), bx.w1(x), by.w0(y), by.w1(y));
        set_med(x, y);
        bool down = true;
        while (true) {
            bool right = false;
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
                update_block(-1, bx.w0(x), bx.w0(x+1), by.w0(y), by.w1(y));
                ++x;
                update_block(+1, bx.w1(x-1), bx.w1(x), by.w0(y), by.w1(y));
            } else if (down) {
                update_block(-1, bx.w0(x), bx.w1(x), by.w0(y), by.w0(y+1));
                ++y;
                update_block(+1, bx.w0(x), bx.w1(x), by.w1(y-1), by.w1(y));
            } else {
                update_block(-1, bx.w0(x), bx.w1(x), by.w1(y-1), by.w1(y));
                --y;
                update_block(+1, bx.w0(x), bx.w1(x), by.w0(y), by.w0(y+1));
            }
            set_med(x, y);
        }
#endif
    }

    inline void update_block(int op, int x0, int x1, int y0, int y1) {
        for (int y = y0; y < y1; ++y) {
            for (int x = x0; x < x1; ++x) {
                wr.update(op, pack(x, y));
            }
        }
    }

    inline void set_med(int x, int y) {
        out[coord(x, y)] = wr.get_med();
    }

    inline R pack(int x, int y) const {
        return static_cast<R>(y * bx.size + x);
    }

    inline int coord(int x, int y) const {
        return (y + by.start) * bx.dim.size + (x + bx.start);
    }

    WindowRank<T,R> wr;
    BDim bx;
    BDim by;
    const T* const in;
    T* const out;
};



template <typename T, typename R>
class MedCalc1D {
public:
    MedCalc1D(int b_, Dim dimx_, const T* in_, T* out_)
        : wr(b_), bx(dimx_), in(in_), out(out_)
    {}

    void run(int bx_)
    {
        bx.set(bx_);
        calc_rank();
        medians();
    }

private:
    void calc_rank() {
        wr.init_start();
        for (int x = 0; x < bx.size; ++x) {
            wr.init_feed(in[coord(x)], pack(x));
        }
        wr.init_finish();
    }

    void medians() {
#ifdef NAIVE
        for (int x = bx.b0; x < bx.b1; ++x) {
            wr.clear();
            update_block(+1, bx.w0(x), bx.w1(x));
            set_med(x);
        }
#else
        wr.clear();
        int x = bx.b0;
        update_block(+1, bx.w0(x), bx.w1(x));
        set_med(x);
        while (x + 1 < bx.b1) {
            if (x >= bx.dim.h) {
                wr.update(-1, pack(x - bx.dim.h));
            }
            ++x;
            if (x + bx.dim.h < bx.size) {
                wr.update(+1, pack(x + bx.dim.h));
            }
            set_med(x);
        }
#endif
    }

    inline void update_block(int op, int x0, int x1) {
        for (int x = x0; x < x1; ++x) {
            wr.update(op, pack(x));
        }
    }

    inline void set_med(int x) {
        out[coord(x)] = wr.get_med();
    }

    inline R pack(int x) const {
        return static_cast<R>(x);
    }

    inline int coord(int x) const {
        return x + bx.start;
    }

    WindowRank<T,R> wr;
    BDim bx;
    const T* const in;
    T* const out;
};


template <typename T, typename R>
void median_filter_impl_2d(int x, int y, int hx, int hy, int b, const T* in, T* out) {
    if (2 * hx + 1 > b || 2 * hy + 1 > b) {
        throw std::invalid_argument("window too large for this block size");
    }
    Dim dimx(b, x, hx);
    Dim dimy(b, y, hy);
    #pragma omp parallel
    {
        MedCalc2D<T,R> mc(b, dimx, dimy, in, out);
        #pragma omp for collapse(2)
        for (int by = 0; by < dimy.count; ++by) {
            for (int bx = 0; bx < dimx.count; ++bx) {
                mc.run(bx, by);
            }
        }
    }
}


template <typename T, typename R>
void median_filter_impl_1d(int x, int hx, int b, const T* in, T* out) {
    if (2 * hx + 1 > b) {
        throw std::invalid_argument("window too large for this block size");
    }
    Dim dimx(b, x, hx);
    #pragma omp parallel
    {
        MedCalc1D<T,R> mc(b, dimx, in, out);
        #pragma omp for
        for (int bx = 0; bx < dimx.count; ++bx) {
            mc.run(bx);
        }
    }
}


template <typename T>
void median_filter_2d(int x, int y, int hx, int hy, int blockhint, const T* in, T* out) {
    int h = std::max(hx, hy);
    int blocksize = blockhint ? blockhint : choose_blocksize_2d(h);
    if (blocksize < 16) {
        median_filter_impl_2d<T,uint8_t>(x, y, hx, hy, blocksize, in, out);
    } else if (blocksize < 256) {
        median_filter_impl_2d<T,uint16_t>(x, y, hx, hy, blocksize, in, out);
    } else {
        median_filter_impl_2d<T,uint32_t>(x, y, hx, hy, blocksize, in, out);
    }
}

template <typename T>
void median_filter_1d(int x, int hx, int blockhint, const T* in, T* out) {
    int blocksize = blockhint ? blockhint : choose_blocksize_1d(hx);
    if (blocksize < 256) {
        median_filter_impl_1d<T,uint8_t>(x, hx, blocksize, in, out);
    } else if (blocksize < 65536) {
        median_filter_impl_1d<T,uint16_t>(x, hx, blocksize, in, out);
    } else {
        median_filter_impl_1d<T,uint32_t>(x, hx, blocksize, in, out);
    }
}

template void median_filter_2d<float>(int x, int y, int hx, int hy, int blockhint, const float* in, float* out);
template void median_filter_2d<double>(int x, int y, int hx, int hy, int blockhint, const double* in, double* out);

template void median_filter_1d<float>(int x, int hx, int blockhint, const float* in, float* out);
template void median_filter_1d<double>(int x, int hx, int blockhint, const double* in, double* out);

