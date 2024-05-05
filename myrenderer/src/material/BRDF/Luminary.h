#pragma once
#ifndef MATERIAL_BRDF_LUMINARY
#define MATERIAL_BRDF_LUMINARY
#include <material/Material.h>
#include <common/utility.h>
#include<texture/SolidColorTexture.h>

class Luminary : public Material
{
public:
    Luminary(){}
    Luminary(const Vec3& emittance) :emittance(make_shared<SolidColorTexture>(emittance)) {}
    Luminary(const shared_ptr<Texture>& emittance) :emittance(emittance) {}

    static ColorN lambert(const double cosNI, const ColorN& albedo) {
        if (cosNI > 0.0f) {
            ColorN diffuse = albedo / pi;   //�������ɢ��  �Ƶ���֪fr_diff=1/pi   ������ɫ(��RGB��ͨ�����������)��Ϊfr_diff=albedo/pi
            return diffuse;
        } else {
            return ColorN(0,0,0);
        }
    }

    virtual bool isLuminary() const override {
        return true;
    }

    virtual ColorN getEmittance(double u = 0,double v=0) const override {
        return emittance->value(u,v);
    }

    virtual Vec3 sampleScatter(const Vec3& wi, const Vec3& wo, const Vec3& normal, double& pdf) const override {
        pdf = _pdf(wi, wo, normal);
        double x_1 = random_double(), x_2 = random_double();
        double z = fabs(1.0 - 2.0 * x_1);
        double r = sqrt(1.0 - z * z), phi = 2 * pi * x_2;
        Vec3 localVec(r * cos(phi), r * sin(phi), z);
        return localToWorld(localVec, normal);

    }

    virtual ColorN eval(const Vec3& wi, const Vec3& wo, const Vec3& normal,double u = 0,double v = 0) const override {
        double cosNI = normal.dot(wi); //wiΪ�������,Ӧ���뷨�߼н�С��90���ܷ���������
        ColorN c = lambert(cosNI, albedo->value(u,v));
        return kd->value(u,v).cwiseProduct(c);
    }

    shared_ptr<Texture> albedo = make_shared<SolidColorTexture>(ColorN(1, 1, 1));
    shared_ptr<Texture> kd = make_shared<SolidColorTexture>(Vec3(1, 1, 1));
    shared_ptr<Texture> emittance;

private:
    double _pdf(const Vec3& wi, const Vec3& wo, const Vec3& normal) const {
        if (wi.dot(normal) > 0.0f) {
            return 0.5f / pi;
        } else {
            return 0.0f;
        }
    }
};


#endif // !MATERIAL_BRDF_LUMINARY
