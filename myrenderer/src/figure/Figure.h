#pragma once
#ifndef FIGURE_FIGURE_H
#define FIGURE_FIGURE_H
#include <device/Canvas.h>

class Figure {
public:
	virtual ~Figure() = default;
	virtual void drawOn(Canvas& canvas) const = 0;
};

#endif // FIGURE_H
