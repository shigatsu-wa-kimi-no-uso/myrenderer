#pragma once
#ifndef COMMON_RAY_H
#define COMMON_RAY_H
#include <common/global.h>

class Ray
{
public:
    Ray() {}
    Ray(const Point3& origin, const Vec3& direction) : origin(origin), direction(direction) {}
    Point3 origin;
    Vec3 direction;
    Point3 operator()(double t) const {
        return at(t);
    }
    //P(t) = A + t*b
    Point3 at(double t) const {
        return origin + t * direction;
    }

};
#endif // COMMON_RAY_H
