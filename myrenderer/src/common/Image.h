#pragma once
#ifndef COMMON_IMAGE_H
#define COMMON_IMAGE_H

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <common/global.h>
#include <external/stb_image.h>
#include <common/Interval.h>

class Image {
private:
	unsigned char* _imageBuffer;
	int _width;
	int _height;
	int _channelCnt;
	int _bytesPerScanline;
	const int _bytesPerPixel = 3; //预期每像素大小
public:
	bool loadImage(const string& filename) {
		_imageBuffer = stbi_load(filename.c_str(), &_width, &_height, &_channelCnt, _bytesPerPixel);
		_bytesPerScanline = _bytesPerPixel * _width;
		return _imageBuffer != nullptr;
	}

	Color255 getPixelColor(int x, int y) const{
		x = Interval(0, _width - 1).clamp(x);
		y = Interval(0, _height - 1).clamp(y);
		unsigned char* p = _imageBuffer + _bytesPerScanline * y + _bytesPerPixel*x;
		return Color255(p[0], p[1], p[2]);
	}

	int getWidth() const{
		return _width;
	}

	int getHeight() const {
		return _height;
	}


};


#endif // COMMON_IMAGE_H
