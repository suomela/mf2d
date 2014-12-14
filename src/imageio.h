#ifndef IMAGEIO_H
#define IMAGEIO_H

#include "driver.h"

template <typename T>
void write_image(const char* filename, Image<T> img);

VDriver* from_image(Settings settings);

#endif
