#ifndef FILTER_H
#define FILTER_H

template <typename T>
void median_filter(int x, int y, int hx, int hy, int blockhint, const T* in, T* out);

#endif
