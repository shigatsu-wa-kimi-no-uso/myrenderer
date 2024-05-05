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
        //���������б�,����ж�ÿ�������Ƿ�������ཻ
        //Interval(ray_t.min, closest_so_far): ���ƹ��ߴ����ĳ���,���ߴ�����ͷ���򳡾�,ͨ��һ��"����"ֻ��"����"���������(t��С),
        //��Զ�����屻�������嵲ס,ͨ��interval�ɺ��Ը����ֱ���ж�Ϊ"δ����"
        //���޻���,�򲻻��޸�rec
        HitRecord tempRec;
        double closestSoFar = range.max;
        for (const shared_ptr<Hittable>& h : _hittables) {
            if (h->hit(r, Interval(range.min, closestSoFar), tempRec)) {
                closestSoFar = tempRec.t;
            }
        }
        if (tempRec.hitAnything) {
            if (!tempRec.material) {
                //Ϊ��ʱ,����meshmodelʹ��ͬһ����
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