#include <cstdlib>
#include <iostream>
#include "driver.h"
#include "imageio.h"

int main(int argc, const char** argv) {
    if (argc != 5) {
        std::cerr << "usage: " << argv[0] << " radius input output-median output-difference" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    int h = atoi(argv[1]);
    const char* source = argv[2];
    const char* target_med = argv[3];
    const char* target_diff = argv[4];

    VDriver* driver = from_image(source);
    int max_h = driver->max_h();
    if (h < 0 || h > max_h) {
        std::cerr << "for this image type, radius has to be between 0 and " << max_h << std::endl;
        delete driver;
        std::exit(EXIT_FAILURE);
    }
    driver->process(h);
    driver->write(target_med);
    driver->diff();
    driver->write(target_diff);
    delete driver;
}
