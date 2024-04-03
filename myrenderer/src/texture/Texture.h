#pragma once
#ifndef TEXTURE_TEXTURE_H
#define TEXTURE_TEXTURE_H
#include <common/global.h>


class Texture {
public:
	Texture(){}
	virtual ~Texture() = default;
	virtual ColorN value(double u, double v) const = 0;
};

#endif // TEXTURE_TEXTURE_H
