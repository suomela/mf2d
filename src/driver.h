#ifndef DRIVER_H
#define DRIVER_H

template <typename T>
struct Image2D {
    int x;
    int y;
    T* p;

    Image2D(int x_, int y_) : x(x_), y(y_), p(0)
    {}

    inline int size() const {
        return x * y;
    }

    inline void alloc() {
        p = new T[size()];
    }
};


template <typename T>
struct Image1D {
    int x;
    T* p;

    Image1D(int x_) : x(x_), p(0)
    {}

    inline int size() const {
        return x;
    }

    inline void alloc() {
        p = new T[size()];
    }
};


struct Settings {
    int h; // Half-window size
    const char* source;
    const char* target_med;
    const char* target_diff;
};


class VDriver {
public:
    virtual void process() = 0;
    virtual void benchmark() = 0;
    virtual ~VDriver() {}
};


template <typename T>
class Driver2D : public VDriver
{
public:
    // Driver will own img.p
    Driver2D(Settings settings_, Image2D<T> img)
        : settings(settings_), in(img), out(img.x, img.y)
    {
        out.alloc();
    }

    ~Driver2D() {
        delete in.p;
        delete out.p;
    }

    void process();
    void benchmark();

private:
    const Settings settings;
    Image2D<T> in;
    Image2D<T> out;
};


template <typename T>
class Driver1D : public VDriver
{
public:
    // Driver will own img.p
    Driver1D(Settings settings_, Image1D<T> img)
        : settings(settings_), in(img), out(img.x)
    {
        out.alloc();
    }

    ~Driver1D() {
        delete in.p;
        delete out.p;
    }

    void process();
    void benchmark();

private:
    const Settings settings;
    Image1D<T> in;
    Image1D<T> out;
};

#endif
