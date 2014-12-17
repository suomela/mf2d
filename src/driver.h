#ifndef DRIVER_H
#define DRIVER_H

template <typename T>
struct Image {
    int x;
    int y;
    T* p;

    Image(int x_, int y_)
        : x(x_), y(y_), p(0)
    {}

    inline int size() const {
        return x * y;
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
class Driver : public VDriver
{
public:
    // Driver will own img.p
    Driver(Settings settings_, Image<T> img)
        : settings(settings_), in(img), out(img.x, img.y)
    {
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
    Image<T> in;
    Image<T> out;
};

#endif
