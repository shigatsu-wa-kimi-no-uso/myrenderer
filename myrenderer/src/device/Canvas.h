#pragma once
#ifndef DEVICE_CANVAS_H
#define DEVICE_CANVAS_H
#include <common/global.h>
#include <common/Interval.h>
#include <common/Color.h>
#include <vector>
#include <iostream>

class Canvas
{
private:
    using FrameBuffer = std::vector<ColorN>;
    FrameBuffer _frameBuffer;
protected:
    uint64_t _mode;
    int _width;
    int _height;
    std::function<void(Canvas&,const ColorN&, size_t)> _draw_method;

    virtual void _setPixelColor(const ColorN& pixel_color, size_t offset) {
        assert(offset < _width * _height && offset >= 0);
        _frameBuffer[offset] = pixel_color;
    }

    virtual void _blendPixelColor(const ColorN& pixel_color, size_t offset) {
        assert(offset < _width * _height && offset >= 0);
        _frameBuffer[offset] += pixel_color;  //超过1.0的强度在写入文件时会clamp,暂未出现问题
    }

public:

    Canvas() :_mode(0),_width(0),_height(0){
        applyConfigs();
    }

    virtual ~Canvas() = default;

    enum Mode{
        COLOR_OVERLAY = 1,
        SSAA = 2
    };

    void setMode(const Mode& a) {
        _mode |= uint64_t(a);
    }

    void clearMode(const Mode& a) {
        _mode ^= uint64_t(a);
    }

    void clearModes() {
        _mode = 0;
    }

    void setModes(uint64_t m) {
        _mode = m;
    }

    virtual void applyConfigs() {
        _draw_method = (_mode & COLOR_OVERLAY) ? &Canvas::_setPixelColor : &Canvas::_blendPixelColor;
    }

    virtual void setCanvasSize(int width,int height) {
        _width = width;
        _height = height;
        _frameBuffer.resize(size_t(_width) * size_t(_height));
    }

    virtual void clearBuffer() {
        std::fill(_frameBuffer.begin(), _frameBuffer.end(), Vec3(0, 0, 0));
    }

    virtual size_t viewportCoord_to_bufferOffset(const Point2i vp) {
        //视口坐标 to buffer偏移,buffer按照图像坐标系,先计算canvas坐标(y从上到下递减),故需翻转y
        //假设视口高度20像素,则坐标范围[0,19],翻转后19->0 0->19,即canvas_y = 20-1-vp_y
        int canvas_y = (_height - 1 - vp.y());
        return _width * canvas_y + vp.x();
    }

    virtual void drawOnePixel(const ColorN& pixel_color, size_t offset) {
        _draw_method(*this, pixel_color, offset);
    }

    virtual void outputPixelBuffer(std::ostream& out) {
        //PPM格式输出
        using namespace std;
        std::cout.flush();
        cout << "P3\n" << _width << ' ' << _height << "\n255\n";
        for (int i = 0; i < _width * _height; i++) {
            const ColorN& pixel_color = _frameBuffer[i];

            //颜色若有多个sample需要在之前平均！此处不负责平均

            // Apply the linear to gamma transform.
            //gamma矫正
          //  ColorN corrected = linear_to_gamma(pixel_color);
            Color255 outputColor = normal_to_RGB255(pixel_color);
            /*
            if (outputColor != Color255(0, 0, 0)) {
                std::cout << "(" << i/_width << "," << i%_width << ")" << "\n";
                std::cout << outputColor[0] << " " << outputColor[1] << " " << outputColor[2] << "\n";
            }*/
            // Write the translated [0,255] value of each color component.
            
            out << outputColor.x() << ' '
                << outputColor.y() << ' '
                << outputColor.z() << '\n';
        }
    }
};

#endif // DEVICE_CANVAS_H