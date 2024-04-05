#pragma once
#ifndef TEXTURE_TEXTURE_H
#define TEXTURE_TEXTURE_H
#include <common/global.h>


class Texture {
public:
	Texture(){}
	virtual ~Texture() = default;
	virtual ColorN value(double u, double v) const {
		return normalizeColor(valueRGB255(u, v));
	}
	virtual Color255 valueRGB255(double u, double v) const {
		return Color255(255, 255, 255);
	}
	virtual int height() const {
		return -1;
	}
	virtual int width() const {
		return -1;
	}
};

#endif // TEXTURE_TEXTURE_H
