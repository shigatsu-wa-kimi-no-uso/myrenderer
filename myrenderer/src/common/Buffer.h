#pragma once
#ifndef COMMON_BUFFER_H
#define COMMON_BUFFER_H
#include <vector>
#include <common/Color.h>

template<typename T>
using Subunits = std::vector<T>;

template<typename T>
struct BufUnit {
    Subunits<T> subunits;
    T& operator[](size_t i) {
        return subunits[i];
    }

    const T& operator[](size_t i) const {
        return subunits[i];
    }
};

using FrameBuffer = std::vector<std::vector<BufUnit<ColorN>>>;
using ZBuffer = std::vector<std::vector<BufUnit<double>>>;

struct ShadowMap {
    shared_ptr<FrameBuffer> buffer = make_shared<FrameBuffer>();
    int width;
    int height;
};

#endif 
