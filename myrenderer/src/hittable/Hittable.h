#pragma once
#ifndef HITTABLE_HITTABLE_H
#define HITTABLE_HITTABLE_H
#include <memory>
#include <vector>
#include <common/global.h>
#include <common/Interval.h>
#include <material/Material.h>



class Hittable
{
public:
    virtual ~Hittable() = default;

    virtual bool hit(const Ray& r, Interval ray_t, HitRecord& rec) const = 0;

    virtual void set_face_normal(const Ray& r, const Vec3& outward_normal, HitRecord& rec) const {
        // Sets the hit record normal vector.
        // NOTE: the parameter `outward_normal` is assumed to have unit length.
        //根据几何体向外的法向量和光线方向向量点积符号判断光线射在了几何体外表面(正面)还是内表面
        rec.front_face = r.direction().dot(outward_normal) < 0;
        rec.normal = rec.front_face ? outward_normal : -outward_normal;
    }
};

class HittableList : public Hittable {
public:
    std::vector<shared_ptr<Hittable>> objects;

    HittableList() {}
    HittableList(shared_ptr<Hittable> object) { add(object); }

    void clear() { objects.clear(); }

    void add(shared_ptr<Hittable> object) {
        objects.push_back(object);
    }

    bool hit(const Ray& r, Interval ray_t, HitRecord& rec) const override {
        HitRecord temp_rec;
        bool hit_anything = false;
        double closest_so_far = ray_t.max;
        //遍历物体列表,逐个判断每个物体是否与光线相交
        //Interval(ray_t.min, closest_so_far): 限制光线传播的长度,光线从摄像头射向场景,通过一条"光线"只能"看见"最近的物体(t最小),
        //较远的物体被近的物体挡住,通过interval可忽略该情况直接判断为"未击中"
        for (const shared_ptr<Hittable>& object : objects) {
            if (object->hit(r, Interval(ray_t.min, closest_so_far), temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hit_anything;
    }
};


#endif // HITTABLE_HITTABLE_H