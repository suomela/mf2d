#include "driver.h"
#include "filter.h"
#include "imageio.h"

template <typename T>
void Driver<T>::process() {
    median_filter<T>(in.x, in.y, settings.h, settings.h, in.p, out.p);
    write_image(settings.target_med, out);
    for (int i {0}; i < out.x * out.y; ++i) {
        out.p[i] = in.p[i] - out.p[i];
    }
    write_image(settings.target_diff, out);
}

template class Driver<float>;
template class Driver<double>;
