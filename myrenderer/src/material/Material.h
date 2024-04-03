#pragma once
#ifndef MATERIAL_MATERIAL_H
#define MATERIAL_MATERIAL_H
#include <common/global.h>
#include <common/Ray.h>
#include <common/HitRecord.h>


//�Բ��ʵ����Բ�ʹ�ö�̬����ȥ��ȡ���������ض�shader��ǿ��ת�����ͻ�ȡ����

class Material
{
public:

    static Vec3 reflect(const Vec3& unit_r_in, const Vec3& normal) {
        return unit_r_in - 2 * unit_r_in.dot(normal) * normal;
    }

    static Vec3 refract(const Vec3& unit_r_in, const Vec3& n, double etai_over_etat) {
        //������ߵ�λ���� unit_r_in
        double cos_theta = std::min(n.dot(-unit_r_in), 1.0);
        Vec3 r_out_perp = etai_over_etat * (unit_r_in + cos_theta * n);
        Vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.squaredNorm())) * n;
        return r_out_perp + r_out_parallel;
    }

    virtual ~Material() = default;

    //���ڲ�ͬ�Ĳ���(Material������)����ȡ��ͬ��scatter������ɢ�䣬���䣬����ȣ�
    virtual bool scatter(const Ray& r_in, const HitRecord& hitRecord, ColorN& attenuation, Ray& scattered) const {
        return false;
    }

};


#endif // MATERIAL_MATERIAL_H