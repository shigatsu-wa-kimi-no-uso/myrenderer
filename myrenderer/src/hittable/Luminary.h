#pragma once
#ifndef HITTABLE_LUMINARY_H
#define HITTABLE_LUMINARY_H
#include <hittable/Hittable.h>

class Luminary : public Hittable {
public:
    Point3 position;

    virtual bool hit(const Ray& r, Interval ray_t, HitRecord& rec) const {
        return false;
    }

    virtual void set_face_normal(const Ray& r, const Vec3& outward_normal, HitRecord& rec) const {
        // Sets the hit record normal vector.
        // NOTE: the parameter `outward_normal` is assumed to have unit length.
        //根据几何体向外的法向量和光线方向向量点积符号判断光线射在了几何体外表面(正面)还是内表面
        rec.front_face = r.direction().dot(outward_normal) < 0;
        rec.normal = rec.front_face ? outward_normal : -outward_normal;
    }


};

#endif // HITTABLE_LUMINARY_H
