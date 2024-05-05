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
    Point3 _pixel00Pos;    //第0个像素对应的采样位置
    Point3 _relativePixel00Pos;    //第0个像素对应的采样位置(相对于摄像头的,即当摄像头位置在(0,0,0)时的)
    Vec3 _viewU; // u朝左, v朝下, 与图像坐标系方向相同
    Vec3 _viewV;
    Vec3   _deltaU;  // 视野窗口u方向每像素采样间隔  
    Vec3   _deltaV;  // 视野窗口v方向每像素采样间隔
    int _lastIndex = 0;
    ColorN _backgroundColor = ColorN(0, 1, 0);

    shared_ptr<HittableContainer> _normalObjs; //非发光体
    shared_ptr<HittableContainer> _luminaries; //发光体
    HittableMap _hittableMap;

public:
    //两个相邻像素中心间的距离为1个单位,一个像素的范围是个边长为1个单位的正方形
    Vec3 sampleInPixelSquare(int u, int v) const {
        return sampleInPixelSquare(u, v, _pixel00Pos, _deltaU, _deltaV);
    }

    Vec3 sampleInPixelSquareRelative(int u, int v) const {
        return sampleInPixelSquare(u, v, _relativePixel00Pos, _deltaU, _deltaV);
    }

    Vec3 sampleInPixelSquare(int u, int v, const Point3& pixel00Pos) const {
        //在一个方形像素中随机取点,起到发射不同光线并反走样的效果
        return sampleInPixelSquare(u, v, pixel00Pos, _deltaU, _deltaV);
    }

    Vec3 sampleInPixelSquare(int u, int v,const Point3& pixel00Pos,const Vec3& deltaU,const Vec3& deltaV) const {
        //在一个方形像素中随机取点,起到发射不同光线并反走样的效果
        Point3 pixelCenter = pixel00Pos + (u * deltaU) + (v * deltaV);
        double px = random_double(-0.5, 0.5);
        double py = random_double(-0.5, 0.5);
        Point3 pixelSample = pixelCenter + px * deltaU + py * deltaV;
        return pixelSample;
    }

    void initialize(const Camera& camera,int imageWidth, int imageHeight) {
        Point3 center = camera.lookfrom; //摄像头中心

        // Determine viewport dimensions.
       // double focal_length = (lookfrom - lookat).norm();//摄像头中心到幕布的距离,又叫焦距
        double theta = deg2rad(camera.fov);
        double h = tan(theta / 2);
        double fieldHeight = 2 * h * camera.focusDist; //摄像机视野长宽
        double fieldWidth = fieldHeight * camera.aspectRatio; //使用输出图像的w/h重新计算aspect_ratio,更精准

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        //
        Vec3 g = (camera.lookat - camera.lookfrom).normalized(); //g轴
        Vec3 z = -g;  //z轴
        Vec3 gxt = camera.vup.cross(z).normalized(); //gxt
        Vec3 t = z.cross(gxt);    //真正的t向量
        _viewU = fieldWidth * gxt;   // +------→ u
        _viewV = fieldHeight * (-t); // |
                                     // ↓v
        //uv轴采样间隔,根据图像长宽确定
        //两种方法:fieldWidth视作从最左边到最右边的像素距离,则n个像素有n-1个距离,_deltaU = _viewU / (imageWidth-1)
        //fieldWidth视作从最左边到最右边的像素个数,则n个像素把总距离分裂成n份,_deltaU = _viewU / imageWidth
        _deltaU = _viewU / imageWidth;
        _deltaV = _viewV / imageHeight;
        Vec3 relativeFieldUpperLeft = (camera.focusDist * (-z)) - _viewU / 2 - _viewV / 2;
        Vec3 fieldUpperLeft = center + relativeFieldUpperLeft;  //从摄像头中心,沿-z轴,沿-u方向, 沿v方向
        _relativePixel00Pos = relativeFieldUpperLeft + 0.5 * (_deltaU + _deltaV);
        _pixel00Pos = fieldUpperLeft + 0.5 * (_deltaU + _deltaV);  // 像素距离边界是半个像素长度
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
