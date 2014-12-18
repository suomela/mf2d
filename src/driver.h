#ifndef DRIVER_H
#define DRIVER_H

template <typename T>
struct Image2D {
    int x;
    int y;
    T* p;

    Image2D() : x(1), y(1), p(0)
    {}

    Image2D(int x_, int y_) : x(x_), y(y_), p(0)
    {}

    void like(Image2D<T> o) {
        x = o.x;
        y = o.y;
    }

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

    Image1D() : x(1), p(0)
    {}

    Image1D(int x_) : x(x_), p(0)
    {}

    void like(Image1D<T> o) {
        x = o.x;
    }

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


template <typename T, typename I>
class Driver : public VDriver
{
public:
    // Driver will own img.p
    Driver(Settings settings_, I img)
        : settings(settings_), in(img)
    {
        out.like(in);
        out.alloc();
    }

    ~Driver() {
        delete in.p;
        delete out.p;
    }

    void process();
    void benchmark();

private:
    const Settings settings;
    I in;
    I out;
};


#endif
