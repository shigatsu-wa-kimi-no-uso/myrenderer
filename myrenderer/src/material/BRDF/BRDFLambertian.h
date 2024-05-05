#pragma once
#ifndef MATERIAL_BRDF_BRDF_LAMBERTIAN_H
#define MATERIAL_BRDF_BRDF_LAMBERTIAN_H
#include<material/material.h>
#include<texture/SolidColorTexture.h>
#include<common/utility.h>

class BRDFLambertian : public Material
{
public:
    BRDFLambertian(){}
    BRDFLambertian(const ColorN& albedo):albedo(make_shared<SolidColorTexture>(albedo)){}
    BRDFLambertian(const ColorN& albedo,const Vec3& kd) :albedo(make_shared<SolidColorTexture>(albedo)),kd(make_shared<SolidColorTexture>(kd)) {}
    BRDFLambertian(const shared_ptr<Texture>& albedo):albedo(albedo){}
    BRDFLambertian(const shared_ptr<Texture>& albedo, const shared_ptr<Texture>& kd) :albedo(albedo),kd(kd) {}

    static ColorN lambert(double cosNI, const ColorN& albedo) {
        if (cosNI > 0.0f) {
            ColorN diffuse = albedo / pi;   //理想均匀散射  推导可知fr_diff=1/pi   考虑颜色(即RGB各通道的吸收情况)后为fr_diff=albedo/pi
            return diffuse;
        } else {
            return ColorN(0,0,0);
        }
    }

    virtual Vec3 sampleScatter(const Vec3& wi, const Vec3& wo, const Vec3& normal,double& pdf) const override {
        double x_1 = random_double(), x_2 = random_double();
        double z = fabs(1.0 - 2.0 * x_1);
        double r = sqrt(1.0 - z * z), phi = 2 * pi * x_2;
        Vec3 localVec(r * cos(phi), r * sin(phi), z);
        Vec3 wi_indir = localToWorld(localVec, normal);
        pdf = 0.5 / pi;
        return wi_indir;

    }

    virtual ColorN eval(const Vec3& wi, const Vec3& wo, const Vec3& normal,double u = 0,double v = 0) const override{
        double cosNI = normal.dot(wi); //wi为入射光线,应该与法线夹角小于90才能发生漫反射
        ColorN c = lambert(cosNI, albedo->value(u,v));
        return kd->value(u,v).cwiseProduct(c);
    }

    shared_ptr<Texture> albedo;
    shared_ptr<Texture> kd = make_shared<SolidColorTexture>(Vec3(1, 1, 1));
  //  ColorN albedo;
   // Vec3 kd = Vec3(1,1,1);

private:
    double _pdf(const Vec3& wi, const Vec3& wo, const Vec3& normal) const {
        if (wi.dot(normal) > 0.0) {
            return 0.5 / pi;
        } else {
            return 0.0;
        }
    }

};

#endif // !MATERIAL_BRDF_BRDF_LAMBERTIAN_H
