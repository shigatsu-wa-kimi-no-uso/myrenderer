#pragma once
#ifndef RAY_H
#define RAY_H
#include <common/global.h>

class Ray
{
private:
    Point3 _origin;
    Vec3 _direction;
public:
    Ray() {}
    Ray(const Point3& origin, const Vec3& direction) : _origin(origin), _direction(direction) {}

    Point3 origin() const { return _origin; }
    Vec3 direction() const { return _direction; }

    //P(t) = A + t*b
    Point3 at(double t) const {
        return _origin + t * _direction;
    }

};
#endif // RAY_H
