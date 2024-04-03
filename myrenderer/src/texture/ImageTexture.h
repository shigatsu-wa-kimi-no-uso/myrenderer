#pragma once
#ifndef TEXTURE_IMAGE_TEXTURE_H
#define TEXTURE_IMAGE_TEXTURE_H
#include <texture/Texture.h>
#include <common/Image.h>

class ImageTexture : public Texture {
private:
	Image _image;
public:
	ImageTexture(const string& filename) {
		setTextureImage(filename);
	}
	ColorN value(double u, double v) const override {
		int x = u * _image.getWidth();
		int y = (1 - v) * _image.getHeight();  //图像坐标系y轴向下！！

		Color255 color = _image.getPixelColor(x, y);
		return normalizeColor(color);
	}

	bool setTextureImage(const string& filename) {
		return _image.loadImage(filename);
	}

};


#endif
