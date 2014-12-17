#include <cstdlib>
#include <iostream>
#include "driver.h"
#include "imageio.h"
#include "param.h"

static int parse_int(const char *s, int min, int max, const char *descr) {
    char *endptr;
    long x = std::strtol(s, &endptr, 10);
    if (s[0] == 0 || endptr[0] != 0) {
        std::cerr << descr << " must be a number" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if (x < min || x > max) {
        std::cerr << descr << " must be between " << min << " and " << max << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return static_cast<int>(x);
}

int main(int argc, const char** argv) {
    if (argc != 5) {
        std::cerr << "usage: " << argv[0] << " radius input output-median output-difference" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    Settings settings;
    settings.h = parse_int(argv[1], 0, MAX_H, "radius");
    settings.source = argv[2];
    settings.target_med = argv[3];
    settings.target_diff = argv[4];
    VDriver* driver = from_image(settings);
    driver->process();
    delete driver;
}
