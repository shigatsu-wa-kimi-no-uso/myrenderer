#pragma once
#ifndef DEVICE_SCENE_H
#define DEVICE_SCENE_H
#include <common/global.h>
#include <common/utility.h>
#include <device/Camera.h>
#include <hittable/HittableList.h>
#include <hittable/BVH.h>

using HittableMap = std::unordered_map<int, shared_ptr<Hittable>>;

class Scene {
    Point3 _center;         // Camera center
    Point3 _pixel00Pos;    //��0�����ض�Ӧ�Ĳ���λ��
    Point3 _relativePixel00Pos;    //��0�����ض�Ӧ�Ĳ���λ��(���������ͷ��,��������ͷλ����(0,0,0)ʱ��)
    Vec3 _viewU; // u����, v����, ��ͼ������ϵ������ͬ
    Vec3 _viewV;
    Vec3   _deltaU;  // ��Ұ����u����ÿ���ز������  
    Vec3   _deltaV;  // ��Ұ����v����ÿ���ز������
    int _lastIndex = 0;
    ColorN _backgroundColor = ColorN(0, 1, 0);

    shared_ptr<HittableContainer> _normalObjs; //�Ƿ�����
    shared_ptr<HittableContainer> _luminaries; //������
    HittableMap _hittableMap;

public:
    //���������������ļ�ľ���Ϊ1����λ,һ�����صķ�Χ�Ǹ��߳�Ϊ1����λ��������
    Vec3 sampleInPixelSquare(int u, int v) const {
        return sampleInPixelSquare(u, v, _pixel00Pos, _deltaU, _deltaV);
    }

    Vec3 sampleInPixelSquareRelative(int u, int v) const {
        return sampleInPixelSquare(u, v, _relativePixel00Pos, _deltaU, _deltaV);
    }

    Vec3 sampleInPixelSquare(int u, int v, const Point3& pixel00Pos) const {
        //��һ���������������ȡ��,�𵽷��䲻ͬ���߲���������Ч��
        return sampleInPixelSquare(u, v, pixel00Pos, _deltaU, _deltaV);
    }

    Vec3 sampleInPixelSquare(int u, int v,const Point3& pixel00Pos,const Vec3& deltaU,const Vec3& deltaV) const {
        //��һ���������������ȡ��,�𵽷��䲻ͬ���߲���������Ч��
        Point3 pixelCenter = pixel00Pos + (u * deltaU) + (v * deltaV);
        double px = random_double(-0.5, 0.5);
        double py = random_double(-0.5, 0.5);
        Point3 pixelSample = pixelCenter + px * deltaU + py * deltaV;
        return pixelSample;
    }

    void initialize(const Camera& camera,int imageWidth, int imageHeight) {
        Point3 center = camera.lookfrom; //����ͷ����

        // Determine viewport dimensions.
       // double focal_length = (lookfrom - lookat).norm();//����ͷ���ĵ�Ļ���ľ���,�ֽн���
        double theta = deg2rad(camera.fov);
        double h = tan(theta / 2);
        double fieldHeight = 2 * h * camera.focusDist; //�������Ұ����
        double fieldWidth = fieldHeight * camera.aspectRatio; //ʹ�����ͼ���w/h���¼���aspect_ratio,����׼

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        //
        Vec3 g = (camera.lookat - camera.lookfrom).normalized(); //g��
        Vec3 z = -g;  //z��
        Vec3 gxt = camera.vup.cross(z).normalized(); //gxt
        Vec3 t = z.cross(gxt);    //������t����
        _viewU = fieldWidth * gxt;   // +------�� u
        _viewV = fieldHeight * (-t); // |
                                     // ��v
        //uv��������,����ͼ�񳤿�ȷ��
        //���ַ���:fieldWidth����������ߵ����ұߵ����ؾ���,��n��������n-1������,_deltaU = _viewU / (imageWidth-1)
        //fieldWidth����������ߵ����ұߵ����ظ���,��n�����ذ��ܾ�����ѳ�n��,_deltaU = _viewU / imageWidth
        _deltaU = _viewU / imageWidth;
        _deltaV = _viewV / imageHeight;
        Vec3 relativeFieldUpperLeft = (camera.focusDist * (-z)) - _viewU / 2 - _viewV / 2;
        Vec3 fieldUpperLeft = center + relativeFieldUpperLeft;  //������ͷ����,��-z��,��-u����, ��v����
        _relativePixel00Pos = relativeFieldUpperLeft + 0.5 * (_deltaU + _deltaV);
        _pixel00Pos = fieldUpperLeft + 0.5 * (_deltaU + _deltaV);  // ���ؾ���߽��ǰ�����س���
    }

    void addHittable(shared_ptr<Hittable> hittable) {
        _hittableMap[_lastIndex++] = hittable;
    }

    void useBVH() {
        _normalObjs = make_shared<BVH>();
        _luminaries = make_shared<BVH>();
    }

    void useHittableList() {
        _normalObjs = make_shared<HittableList>();
        _luminaries = make_shared<HittableList>();
    }


    void build() {
        _luminaries->clear();
        _normalObjs->clear();
        for (const pair<const int, shared_ptr<Hittable>>& entry : _hittableMap) {
            const shared_ptr<Hittable>& hittable = entry.second;
            if (hittable->material->isLuminary()) {
                _luminaries->add(hittable);
            } else {
                _normalObjs->add(hittable);
            }
        }
        _luminaries->build();
        _normalObjs->build();
    }

    [[deprecated]]
    HitRecord __trace(const Ray& ray, const Interval& range) const {
        HitRecord hitRecord;
        hitRecord.hitAnything = false;
        double closestSoFar = range.max;
        for (const pair<const int, shared_ptr<Hittable>>& item : _hittableMap) {
            shared_ptr<Hittable> obj = item.second;
            if (obj->hit(ray, Interval(range.min, closestSoFar), hitRecord)) {
                closestSoFar = hitRecord.t;
            }
        }
        return hitRecord;
    }

    HitRecord trace(const Ray& ray, const Interval& range) const {
        HitRecord rec;
        _normalObjs->hit(ray, range, rec);
        _luminaries->hit(ray, Interval(range.min, rec.t), rec);
        return rec;
    }

    [[deprecated]]
    double __getLuminaryArea() const {
        double areaSum = 0;
        for (const pair<const int, shared_ptr<Hittable>>& item : _hittableMap) {
            shared_ptr<Hittable> obj = item.second;
            if (obj->material->isLuminary()) {
                areaSum += obj->getArea();
            }
        }
        return areaSum;
    }

    double getLuminaryArea() const {
        return _luminaries->getArea();
    }

    [[deprecated]]
    void __sampleLight(HitRecord& rec, double& pdf) const {
        double areaSum = getLuminaryArea();
        double p = random_double() * areaSum;
        areaSum = 0;
        for (const pair<const int, shared_ptr<Hittable>>& item : _hittableMap) {
            shared_ptr<Hittable> obj = item.second;
            if (obj->material->isLuminary()) {
                areaSum += obj->getArea();
                if (p <= areaSum) {
                    obj->samplePoint(rec, pdf);
                    break;
                }
            }
        }
    }

    void sampleLight(HitRecord& rec, double& pdf) const {
        _luminaries->samplePoint(rec, pdf);
    }
};

#endif 
