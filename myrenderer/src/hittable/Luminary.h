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
        //���ݼ���������ķ������͹��߷���������������жϹ��������˼����������(����)�����ڱ���
        rec.front_face = r.direction().dot(outward_normal) < 0;
        rec.normal = rec.front_face ? outward_normal : -outward_normal;
    }


};

#endif // HITTABLE_LUMINARY_H
