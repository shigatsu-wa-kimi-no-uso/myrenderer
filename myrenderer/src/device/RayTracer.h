#pragma once
#ifndef DEVICE_RAY_TRACER_H
#define DEVICE_RAY_TRACER_H
#include <device/Camera.h>
#include <device/Scene.h>
#include <common/Ray.h>
#include <common/utility.h>

class RayTracer {
private:
    double _russianRoulette = 0.8;
    shared_ptr<Camera> _camera;
    shared_ptr<Scene> _scene;

public:
    void initialize(const shared_ptr<Camera>& camera, const shared_ptr<Scene>& scene,int width, int height) {
        _camera = camera;
        _scene = scene;
        _scene = make_shared<Scene>();
        _scene->initialize(*camera, width, height);

    }

    void addHittable(const shared_ptr<Hittable>& hittable) {
        _scene->addHittable(hittable);
    }

    Ray getRay(const Point2i& pixelPos) const {
        Point3 pixelSample = _scene->sampleInPixelSquare(pixelPos[0], pixelPos[1]);
        //要实现blur,需多次采样
        Point3 rayOrigin = _camera->sampleInDefocusDisc(); //光线出发点:镜片上的某个点的采样(若camera._defocusAngle为0,则为镜片中心)
        Vec3 rayDirection = pixelSample - rayOrigin; //从摄像头中心指向场景
        rayDirection.normalize();
        return Ray(rayOrigin, rayDirection);
    }


    Ray getRay2(const Point2i& pixelPos) const {
        Point3 pixelSample = _scene->sampleInPixelSquareRelative(pixelPos[0], pixelPos[1]);
        //要实现blur,需多次采样
        Point3 rayOrigin = _camera->sampleInDefocusDisc(); //光线出发点:镜片上的某个点的采样(若camera._defocusAngle为0,则为镜片中心)
        Vec3 rayDirection = pixelSample; //假定从(0,0,0)射出指向pixel,但在之后将向量移动到从camera中采样得到的rayOrigin,更灵活地控制光线方向,避免数字过大时丢失浮点数精度
        return Ray(rayOrigin, rayDirection.normalized());
    }

    HitRecord trace(const Ray& ray) const {
        return trace(ray, Interval(0, infinity));
    }

    HitRecord trace(const Ray& ray,const Interval& range) const{
        return _scene->trace(ray, range);
    }

    void useBVH() const {
        _scene->useBVH();
    }

    void useHittableList() const {
        _scene->useHittableList();
    }

    void build() {
        _scene->build();
    }

    ColorN shade(const HitRecord& hit,const Vec3& wo) {
        ColorN l_dir(0, 0, 0);
        //1)对光源采样,随机获得一条入射光线
        HitRecord l_hit;
        double l_pdf;
        _scene->sampleLight(l_hit, l_pdf);
        
        Vec3 n = hit.normal.normalized();
        Point3 p = hit.point;
        Vec3 x = l_hit.point;
        Vec3 nn = l_hit.normal.normalized();
        ColorN emit = l_hit.material->getEmittance();
        //缓解精度问题导致的自遮挡问题
        Point3 p_deviation;
        if (hit.frontFace) {
            p_deviation = p + 0.001 * n;
        } else {
            p_deviation = p - 0.001 * n;
        }
        Vec3 incidentVec = p_deviation - x; //从光源指向反射点
        Vec3 wi = (-incidentVec).normalized();
        //2)遮挡检测
        bool noDir = false;
        HitRecord obstructionTest = trace(Ray(p_deviation, wi), Interval(0.001, infinity)); //注意调整方向
        if (obstructionTest.hitAnything == true && obstructionTest.material->isLuminary()) {
            //3)立体角->面积微元
            bool noDir = true;
            double dist = incidentVec.norm();
            double dist2 = dist * dist;
            incidentVec = incidentVec.normalized();
            double cos_theta1 = incidentVec.dot(nn);
            double cos_theta = wi.dot(n);

            //4)计算直接光照
            Vec3 fr = hit.material->eval(wi, wo, n);
            l_dir = emit.cwiseProduct( fr) * cos_theta * cos_theta1 / dist2;
            l_dir = l_dir / l_pdf;
        } 


        //5)计算间接光照
        ColorN l_indir(0, 0, 0);
        if (random_double() <= _russianRoulette) {
            double obj_pdf;
            Vec3 wi_indir = hit.material->sampleScatter(wi,wo, n,obj_pdf); //返回的是朝外(与法线夹角小于90)的向量
            HitRecord nextHit = trace(Ray(p_deviation, wi_indir),Interval(0.001,infinity));
            if (nextHit.hitAnything && nextHit.material->isLuminary() == false) {
     
                ColorN c = shade(nextHit, -wi_indir);
                Vec3 fr = hit.material->eval(wi_indir, wo, n);
                 //theta
            
                double theta = wi_indir.dot(n);
                //防止pdf过小导致放大光强
                if (obj_pdf > 0.001) {
                    l_indir = c.cwiseProduct(fr) * theta / obj_pdf;
                }
                /*
                if (noDir == false && l_indir(0)<0.05 && l_indir(1) < 0.05 && l_indir(2) < 0.05
                     && c(0)>0.05 && c(1)>0.05 && c(2)>0.05 && py>334 && py<667 && px>218 && px<241) {
                    std::clog << "no dir,with indir\n";
                    std::clog << "fr:" << fr(0) << " " << fr(1) << " " << fr(2) << "\n";
                    std::clog << "c:" << c(0) << " " <<c(1) << " " << c(2) << "\n";
                    std::clog << "theta:"<<theta<<"\n";
                    std::clog << "pdf:" << obj_pdf << "\n";
                    std::clog << "l_indir:" << l_indir(0) << " " << l_indir(1) << " " << l_indir(2) << "\n";
                }*/
            //    std::clog << "fr:" << fr(0) << " " << fr(1) << " " << fr(2) << "\n";
            }
        }

        return hit.material->getEmittance() + l_dir + l_indir / _russianRoulette; //不要漏了自发光项
    }

    size_t field_to_bufferOffset(int width, int height, int vx, int vy) {
        //视口坐标 to buffer偏移,buffer按照图像坐标系,先计算canvas坐标(y从上到下递减),故需翻转y
        //假设视口高度20像素,则坐标范围[0,19],翻转后19->0 0->19,即canvas_y = 20-1-vp_y
        int canvas_y = (height - 1 - vy);
        return size_t(width) * size_t(canvas_y) + size_t(vx);
    }

};

#endif // DEVICE_RAY_TRACER_H
