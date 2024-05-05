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
        //Ҫʵ��blur,���β���
        Point3 rayOrigin = _camera->sampleInDefocusDisc(); //���߳�����:��Ƭ�ϵ�ĳ����Ĳ���(��camera._defocusAngleΪ0,��Ϊ��Ƭ����)
        Vec3 rayDirection = pixelSample - rayOrigin; //������ͷ����ָ�򳡾�
        rayDirection.normalize();
        return Ray(rayOrigin, rayDirection);
    }


    Ray getRay2(const Point2i& pixelPos) const {
        Point3 pixelSample = _scene->sampleInPixelSquareRelative(pixelPos[0], pixelPos[1]);
        //Ҫʵ��blur,���β���
        Point3 rayOrigin = _camera->sampleInDefocusDisc(); //���߳�����:��Ƭ�ϵ�ĳ����Ĳ���(��camera._defocusAngleΪ0,��Ϊ��Ƭ����)
        Vec3 rayDirection = pixelSample; //�ٶ���(0,0,0)���ָ��pixel,����֮�������ƶ�����camera�в����õ���rayOrigin,�����ؿ��ƹ��߷���,�������ֹ���ʱ��ʧ����������
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
        //1)�Թ�Դ����,������һ���������
        HitRecord l_hit;
        double l_pdf;
        _scene->sampleLight(l_hit, l_pdf);
        
        Vec3 n = hit.normal.normalized();
        Point3 p = hit.point;
        Vec3 x = l_hit.point;
        Vec3 nn = l_hit.normal.normalized();
        ColorN emit = l_hit.material->getEmittance();
        //���⾫�����⵼�µ����ڵ�����
        Point3 p_deviation;
        if (hit.frontFace) {
            p_deviation = p + 0.001 * n;
        } else {
            p_deviation = p - 0.001 * n;
        }
        Vec3 incidentVec = p_deviation - x; //�ӹ�Դָ�����
        Vec3 wi = (-incidentVec).normalized();
        //2)�ڵ����
        bool noDir = false;
        HitRecord obstructionTest = trace(Ray(p_deviation, wi), Interval(0.001, infinity)); //ע���������
        if (obstructionTest.hitAnything == true && obstructionTest.material->isLuminary()) {
            //3)�����->���΢Ԫ
            bool noDir = true;
            double dist = incidentVec.norm();
            double dist2 = dist * dist;
            incidentVec = incidentVec.normalized();
            double cos_theta1 = incidentVec.dot(nn);
            double cos_theta = wi.dot(n);

            //4)����ֱ�ӹ���
            Vec3 fr = hit.material->eval(wi, wo, n);
            l_dir = emit.cwiseProduct( fr) * cos_theta * cos_theta1 / dist2;
            l_dir = l_dir / l_pdf;
        } 


        //5)�����ӹ���
        ColorN l_indir(0, 0, 0);
        if (random_double() <= _russianRoulette) {
            double obj_pdf;
            Vec3 wi_indir = hit.material->sampleScatter(wi,wo, n,obj_pdf); //���ص��ǳ���(�뷨�߼н�С��90)������
            HitRecord nextHit = trace(Ray(p_deviation, wi_indir),Interval(0.001,infinity));
            if (nextHit.hitAnything && nextHit.material->isLuminary() == false) {
     
                ColorN c = shade(nextHit, -wi_indir);
                Vec3 fr = hit.material->eval(wi_indir, wo, n);
                 //theta
            
                double theta = wi_indir.dot(n);
                //��ֹpdf��С���·Ŵ��ǿ
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

        return hit.material->getEmittance() + l_dir + l_indir / _russianRoulette; //��Ҫ©���Է�����
    }

    size_t field_to_bufferOffset(int width, int height, int vx, int vy) {
        //�ӿ����� to bufferƫ��,buffer����ͼ������ϵ,�ȼ���canvas����(y���ϵ��µݼ�),���跭תy
        //�����ӿڸ߶�20����,�����귶Χ[0,19],��ת��19->0 0->19,��canvas_y = 20-1-vp_y
        int canvas_y = (height - 1 - vy);
        return size_t(width) * size_t(canvas_y) + size_t(vx);
    }

};

#endif // DEVICE_RAY_TRACER_H
