#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include "fitsio.h"
#include "imageio.h"


template <typename T> constexpr const char* get_type_descr();
template <> constexpr const char* get_type_descr<float>() { return "32-bit floats"; }
template <> constexpr const char* get_type_descr<double>() { return "64-bit floats"; }

template <typename T> constexpr int get_fits_type();
template <> constexpr int get_fits_type<float>() { return TFLOAT; }
template <> constexpr int get_fits_type<double>() { return TDOUBLE; }

template <typename T> constexpr int get_fits_bitpix();
template <> constexpr int get_fits_bitpix<float>() { return FLOAT_IMG; }
template <> constexpr int get_fits_bitpix<double>() { return DOUBLE_IMG; }

static void fcheck(int s) {
    if (s) {
        fits_report_error(stderr, s);
        std::exit(EXIT_FAILURE);
    }
}


static void verify_dim(const long* naxes) {
    int64_t x {naxes[1]};
    int64_t y {naxes[0]};
    if (x < 1 || y < 1) {
        std::cerr << "image dimensions "
            << x << "x" << y << " too small" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if (x * y >= std::numeric_limits<int>::max()) {
        std::cerr << "image dimensions "
            << x << "x" << y << " too large" << std::endl;
        std::exit(EXIT_FAILURE);
    }
}


template <typename T>
static Image<T> read_image_data(const char* filename, fitsfile* f) {
    int s {0};
    int naxis {0};
    fcheck(fits_get_img_dim(f, &naxis, &s));
    if (naxis != 2) {
        std::cerr << "expected 2-dimensional data, got "
            << naxis << "-dimensional data" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    long naxes[2] {0,0};
    fcheck(fits_get_img_size(f, 2, naxes, &s));
    verify_dim(naxes);
    int x {static_cast<int>(naxes[1])};
    int y {static_cast<int>(naxes[0])};
    Image<T> img(x, y);
    T nulval {std::numeric_limits<T>::quiet_NaN()};
    int anynul {0};
    img.alloc();
    fcheck(fits_read_img(f, get_fits_type<T>(), 1, img.size(), &nulval, img.p, &anynul, &s));
    fcheck(fits_close_file(f, &s));
#ifdef VERBOSE
    int nancount {0};
    if (anynul) {
        for (int i {0}; i < img.size(); ++i) {
            nancount += std::isnan(img.p[i]);
        }
    }
    std::cout << filename
        << ": " << x << "x" << y
        << ", " << get_type_descr<T>()
        << ", " << nancount << " undefined elements"
        << "\n";
#endif
    return img;
}


static fitsfile* open_image_for_reading(const char* filename) {
    fitsfile* f {NULL};
    int s {0};
    fcheck(fits_open_file(&f, filename, READONLY, &s));
    int hdutype {0};
    fcheck(fits_get_hdu_type(f, &hdutype, &s));
    if (hdutype != IMAGE_HDU) {
        std::cerr << "expected IMAGE_HDU, got ";
        if (hdutype == ASCII_TBL) {
            std::cerr << "ASCII_TBL";
        } else if (hdutype == BINARY_TBL) {
            std::cerr << "BINARY_TBL";
        } else {
            std::cerr << hdutype;
        }
        std::cerr << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return f;
}


template <typename T>
static VDriver* from_image_helper(Settings settings, fitsfile* f) {
    Image<T> img {read_image_data<T>(settings.source, f)};
    return new Driver<T>(settings, img);
}


VDriver* from_image(Settings settings) {
    fitsfile* f {open_image_for_reading(settings.source)};
    int s {0};
    int bitpix {0};
    fcheck(fits_get_img_type(f, &bitpix, &s));
    if (bitpix == get_fits_bitpix<float>()) {
        return from_image_helper<float>(settings, f);
    } else if (bitpix == get_fits_bitpix<double>()) {
        return from_image_helper<double>(settings, f);
    } else {
        std::cerr << "unexpected data type: ";
        if (bitpix < 0) {
            std::cerr << (-bitpix) << "-bit floats";
        } else {
            std::cerr << bitpix << "-bit integers";
        }
        std::cerr << std::endl;
        std::exit(EXIT_FAILURE);
    }
}


template <typename T>
void write_image(const char* filename, Image<T> img)
{
    fitsfile* f {NULL};
    int s {0};
    fcheck(fits_create_file(&f, filename, &s));
    long naxes[2] { img.y, img.x };
    fcheck(fits_create_img(f, get_fits_bitpix<T>(), 2, naxes, &s));
    T nulval {std::numeric_limits<T>::quiet_NaN()};
    fcheck(fits_write_imgnull(f, get_fits_type<T>(), 1, img.x * img.y, img.p, &nulval, &s));
    fcheck(fits_close_file(f, &s));
#ifdef VERBOSE
    std::cout << filename
        << ": " << img.x << "x" << img.y
        << ", " << get_type_descr<T>()
        << "\n";
#endif
}


template void write_image(const char* filename, Image<float> img);
template void write_image(const char* filename, Image<double> img);
