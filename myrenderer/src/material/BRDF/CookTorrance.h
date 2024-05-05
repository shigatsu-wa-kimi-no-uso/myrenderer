#pragma once
#ifndef MATERIAL_BRDF_COOK_TORRANCE_H
#define MATERIAL_BRDF_COOK_TORRANCE_H
#include <material/Material.h>
#include <common/utility.h>
#include<texture/SolidColorTexture.h>

class CookTorrance : public Material
{
public:
    CookTorrance() {}
    ~CookTorrance() {}
    CookTorrance(double roughtness,const ColorN& albedo, double metalness):roughness(roughness) {
        this->albedo = make_shared<SolidColorTexture>(albedo * (1 - metalness));
        this->f0 = make_shared<SolidColorTexture>(lerp(ColorN(0.04, 0.04, 0.04), albedo, metalness));
    }
    CookTorrance(double roughtness, const ColorN& f0, double metalness) :roughness(roughness) {
        this->f0 = make_shared<SolidColorTexture>(std::max(f0, ColorN(0.04, 0.04, 0.04)));
        this->albedo = make_shared<SolidColorTexture>((this->f0->value(0,0) - ColorN(0.04, 0.04, 0.04) * (1 - metalness)) / metalness);
    }

    CookTorrance(double roughtness, const shared_ptr<Texture>& albedo, const shared_ptr<Texture>& f0) :roughness(roughness),albedo(albedo),f0(f0) {
    }

    CookTorrance(double roughtness, const ColorN& albedo,const ColorN& f0) 
        :roughness(roughness),
        albedo(make_shared<SolidColorTexture>(albedo)),
        f0(make_shared<SolidColorTexture>(f0)) {}
    CookTorrance(double roughtness, const ColorN& albedo, const ColorN& f0,const Vec3& kd) 
        :roughness(roughness), 
        albedo(make_shared<SolidColorTexture>(albedo)),
        f0(make_shared<SolidColorTexture>(f0)), 
        kd(make_shared<SolidColorTexture>(kd)) {}


    static double normalDistribution(double roughness, double cosNH) {
        //Trowbridge-Reitz GGX
        double alpha2 = roughness * roughness;
        double cosNH2 = cosNH * cosNH;
        double sinNH2 = 1 - cosNH2;
        double denominator = alpha2 * cosNH2 + sinNH2;
        denominator *= pi * denominator;
        return alpha2 / denominator;
    }


    static double schlickGGX(double cosNV, double roughness) {
        //Schlick GGX
        double alphap1 = (roughness + 1);
        double alphap12 = alphap1 * alphap1;
        double kDir = alphap12 / 8.0;
        double denominator = cosNV * (1 - kDir) + kDir;
        if (denominator < 0.001) {
            return 1;
        }
        return cosNV / denominator;
    }


    static double geometry(double cosNR, double cosNI, double roughness) {
        //Shadowing-masking
        return schlickGGX(cosNI, roughness) * schlickGGX(cosNR, roughness);
    }


    static ColorN fresnelSchlick(const ColorN& f0, double cosNI) {
        double t = 1 - cosNI;
        double t2 = t * t;
        double t5 = t2 * t2 * t;
        return f0 + (ColorN(1,1,1) - f0) * t5;
    }

    static ColorN lambert(double cosNI, const ColorN& albedo) {
        if (cosNI > 0.0f) {
            ColorN diffuse = albedo / pi;   //理想均匀散射  推导可知fr_diff=1/pi   考虑颜色(即RGB各通道的吸收情况)后为fr_diff=albedo/pi
            return diffuse;
        } else {
            return ColorN(0, 0, 0);
        }
    }


    //注意数值计算问题！！
    static ColorN cookTorrance(const Vec3& albedo, const Vec3& f0, const ColorN& kd, double cosNI, double cosNR, double cosNH, double roughness) {
        //颜色:漫反射albedo 镜面反射f0
        //能量分配:漫反射kd 镜面发射ks
        cosNI = std::max(cosNI, 0.0);
        cosNR = std::max(cosNR, 0.0);
        cosNH = std::max(cosNH, 0.0);
        ColorN fd = lambert(cosNI, albedo);
        double d = normalDistribution(roughness, cosNH);
        double g = geometry(cosNR, cosNI, roughness);
        double denominator = 4 * cosNR * cosNI;
        if (denominator == 0) {
            denominator = 0.000001; //防止除0出现inf -inf 和nan
        }
        ColorN f = fresnelSchlick(f0, cosNI);
        Vec3 ks_balance = Vec3(1,1,1) - kd; //可以用kd,ks=1-kd平衡能量,也可以使用f代替ks,kd=1-f来保持能量守恒
        ColorN diff = kd.cwiseProduct(fd);
        ColorN spec = ks_balance.cwiseProduct((d * f * g)) / denominator;
        ColorN c = spec + diff;
        return c;
    }


    virtual Vec3 sampleScatter(const Vec3& wi, const Vec3& wo, const Vec3& normal, double& pdf) const override {
        double x_1 = random_double(), x_2 = random_double();
        double z = fabs(1.0 - 2.0 * x_1);
        double r = sqrt(1.0 - z * z), phi = 2 * pi * x_2;
        Vec3 localVec(r * cos(phi), r * sin(phi), z);
        Vec3 wi_indir = localToWorld(localVec, normal);
        pdf = 0.5 / pi;
        return wi_indir;

    }

    virtual ColorN eval(const Vec3& wi, const Vec3& wo, const Vec3& normal, double u = 0, double v = 0) const override {
        double cosNR = normal.dot(wo);
        double cosNI = normal.dot(wi);
        double cosNH = normal.dot((wi + wo).normalized());
        return cookTorrance(albedo->value(u,v), f0->value(u,v), kd->value(u,v), cosNI, cosNR, cosNH, roughness);
    }

    shared_ptr<Texture> albedo;
    shared_ptr<Texture> f0;
    shared_ptr<Texture> kd = make_shared<SolidColorTexture>(Vec3(1, 1, 1));

   // ColorN albedo;
   // ColorN f0;
   // Vec3 kd = Vec3(1, 1, 1);
    double roughness;
private:
    double _pdf(const Vec3& wi, const Vec3& wo, const Vec3& normal) const {
        if (wi.dot(normal) > 0.0) {
            return 0.5 / pi;
        } else {
            return 0.0;
        }
    }

};


#endif // !MATERIAL_BRDF_COOK_TORRANCE_H
