#pragma once
#ifndef TEXTURE_IMAGE_TEXTURE_H
#define TEXTURE_IMAGE_TEXTURE_H
#include <texture/Texture.h>
#include <common/Image.h>

class ImageTexture : public Texture {
private:
	Image _image;
	Interval _wInterval;
	Interval _hInterval;
public:
	ImageTexture(const string& filename) {
		if (setTextureImage(filename)) {
			_wInterval = Interval(0, _image.getWidth() - 1);
			_hInterval = Interval(0, _image.getHeight() - 1);
		}
		
	}
	ColorN value(double u, double v) const override {
		return normalizeColor(valueRGB255(u, v));
	}

	Color255 valueRGB255(double u, double v) const override {
		int x = u * _image.getWidth();
		int y = (1 - v) * _image.getHeight();  //图像坐标系y轴向下！！
		x = _wInterval.clamp(x);
		y = _hInterval.clamp(y);
		Color255 color = _image.getPixelColor(x, y);
		return color;
	}

	bool setTextureImage(const string& filename) {
		return _image.loadImage(filename);
	}

	int height() const {
		return _image.getHeight();
	}
	int width() const {
		return _image.getWidth();
	}

};


#endif
