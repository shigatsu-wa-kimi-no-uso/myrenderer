#pragma once
#ifndef HITTABLE_LUMINARY_H
#define HITTABLE_LUMINARY_H
#include <hittable/Hittable.h>


class Luminary : public Hittable {
public:
    Point3 position;

    virtual bool hit(const Ray& r, const Interval& range, HitRecord& rec) const {
        return false;
    }

};

#endif // HITTABLE_LUMINARY_H
