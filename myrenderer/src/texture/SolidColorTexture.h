#pragma once
#ifndef TEXTURE_SOLID_COLOR_TEXTURE_H
#define TEXTURE_SOLID_COLOR_TEXTURE_H
#include <texture/Texture.h>

class SolidColorTexture : public Texture{
private:
	ColorN _color;
public:
	SolidColorTexture(const ColorN& color) :_color(color) {

	}

	ColorN value(double u, double v) const override {
		return _color;
	}

	void setColor(const Color255& color) {
		_color = normalizeColor(color);
	}

};

#endif // TEXTURE_SOLID_COLOR_TEXTURE_H
