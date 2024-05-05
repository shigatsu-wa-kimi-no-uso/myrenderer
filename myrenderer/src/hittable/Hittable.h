#pragma once
#ifndef HITTABLE_HITTABLE_H
#define HITTABLE_HITTABLE_H
#include <memory>
#include <vector>
#include <common/global.h>
#include <common/Interval.h>
#include <material/Material.h>
#include <common/HitRecord.h>
#include <common/Bounds3.h>

class Hittable
{
public:
    shared_ptr<Material> material;

    virtual ~Hittable() = default;

    virtual bool hit(const Ray& r, const Interval& range, HitRecord& rec) const = 0;

    virtual void set_face_normal(const Ray& r, const Vec3& outward_normal, HitRecord& rec) const {
        // Sets the hit record normal vector.
        // NOTE: the parameter `outward_normal` is assumed to have unit length.
        //根据几何体向外的法向量和光线方向向量点积符号判断光线射在了几何体外表面(正面)还是内表面
        rec.frontFace = r.direction.dot(outward_normal) < 0;
        rec.normal = rec.frontFace ? outward_normal : -outward_normal;
    }

    virtual void samplePoint(HitRecord& rec,double& pdf) const = 0;

    virtual double getArea() const = 0;

    virtual void calculateOthers() {

    }

    virtual Bounds3 getBoundingBox() const {
        return Bounds3();
    }
};


#endif // HITTABLE_HITTABLE_H