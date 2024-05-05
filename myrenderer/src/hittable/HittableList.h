#pragma once
#ifndef HITTABLE_HITTABLE_LIST_H
#define HITTABLE_HITTABLE_LIST_H
#include <hittable/HittableContainer.h>
#include <common/utility.h>

class HittableList : public HittableContainer {
protected:
    std::vector<shared_ptr<Hittable>> _hittables;
public:
    double areaSum;

    HittableList():areaSum(0) {}
    HittableList(shared_ptr<Hittable> object) { add(object); }

    void clear() override { 
        _hittables.clear(); 
    }

    double getArea() const override{
        return areaSum;
    }

    void add(shared_ptr<Hittable> object) override{
        _hittables.push_back(object);
        areaSum += object->getArea();
    }

    bool hit(const Ray& r, const Interval& range, HitRecord& rec) const override {
        //遍历物体列表,逐个判断每个物体是否与光线相交
        //Interval(ray_t.min, closest_so_far): 限制光线传播的长度,光线从摄像头射向场景,通过一条"光线"只能"看见"最近的物体(t最小),
        //较远的物体被近的物体挡住,通过interval可忽略该情况直接判断为"未击中"
        //若无击中,则不会修改rec
        HitRecord tempRec;
        double closestSoFar = range.max;
        for (const shared_ptr<Hittable>& h : _hittables) {
            if (h->hit(r, Interval(range.min, closestSoFar), tempRec)) {
                closestSoFar = tempRec.t;
            }
        }
        if (tempRec.hitAnything) {
            if (!tempRec.material) {
                //为空时,整个meshmodel使用同一材质
                tempRec.material = material;
            }
            rec = tempRec;
            return true;
        }
        return false;
    }

    void samplePoint(HitRecord& rec, double& pdf) const override {
        double p = sqrt(random_double()) * getArea();
        double areaSum = 0;
        for (const shared_ptr<Hittable>& h : _hittables) {
            areaSum += h->getArea();
            if (areaSum > p) {
                h->samplePoint(rec, pdf);
                if (!rec.material) {
                    rec.material = material;
                }
                pdf = 1 / getArea();
                return;
            }
        }
    }
};

#endif // !HITTABLE_HITTABLE_LIST_H