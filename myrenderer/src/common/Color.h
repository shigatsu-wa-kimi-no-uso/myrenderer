#pragma once
#ifndef COMMON_COLOR_H
#define COMMON_COLOR_H
#include <common/global.h>
#include <common/Interval.h>

using ColorN = Eigen::Vector3d;
using Color255 = Eigen::Vector3i;

inline ColorN normalizeColor(const Color255& c) {
    ColorN cn(c.x() / 255.0, c.y() / 255.0, c.z() / 255.0);
    return cn;
}

inline double linear_to_gamma(double linear_component)
{
    return pow(linear_component,0.6);
}

inline ColorN linear_to_gamma(ColorN linear_component)
{
    return ColorN(linear_to_gamma(linear_component.x()), linear_to_gamma(linear_component.y()), linear_to_gamma(linear_component.z()));
}

inline Color255 normal_to_RGB255(const ColorN& normColor) {
    static const Interval intensity(0.0, 1.0);
    return Color255(255 * intensity.clamp(normColor.x()), 255 * intensity.clamp(normColor.y()), 255 * intensity.clamp(normColor.z()));
}

#endif // COMMON_COLOR_H
