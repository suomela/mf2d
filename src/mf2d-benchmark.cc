#include <cstdlib>
#include <iostream>
#include "driver.h"
#include "imageio.h"

int main(int argc, const char** argv) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " input" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    Settings settings;
    settings.h = 0;
    settings.source = argv[1];
    settings.target_med = NULL;
    settings.target_diff = NULL;
    VDriver* driver = from_image(settings);
    driver->benchmark();
    delete driver;
}
