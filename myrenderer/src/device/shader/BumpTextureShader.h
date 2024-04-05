#pragma once
#ifndef DEVICE_SHADER_BUMP_TEXTURE_SHADER_H
#define DEVICE_SHADER_BUMP_TEXTURE_SHADER_H
#include <device/shader/TextureShader.h>
#include <device/shader/NormalShader.h>
#include <material/BlinnPhongWithBump.h>

class BumpTextureShader : public TextureShader{

    virtual void shadeVertex(int nthVertex, Vec4& clip_coord) override {
        Mat4 mv = uni.view * uni.model;
        Mat4 mvIT = mv.inverse().transpose();  //变换过程中要进行法向量矫正！
        Vec4 vertex_view_coord = mv * toVec4(attr.vertexCoord, 1);
        Vec3 normal = toVec3(mvIT * toVec4(attr.normal, 0));;
        a2v.normals[nthVertex] = normal;
        a2v.textureCoords[nthVertex] = attr.textureCoord;
        Interpolator::vertex_view_coords[nthVertex] = toVec3(vertex_view_coord);
        a2v.vertexCoords[nthVertex] = Interpolator::vertex_view_coords[nthVertex];
        clip_coord = uni.projection * vertex_view_coord;
        Vec3 tangent = toVec3((mv*toVec4(attr.tangent,0))).normalized();    //切线不需要矫正,线性良好
        Vec3 bitangent = normal.cross(tangent).normalized();
        v2f_common.tbn << tangent, bitangent, normal.normalized();
    }

    virtual void processVarying(const Vec3& bar) override {
        v2f.fragCoord = Interpolator::viewspace_interpolate(bar, a2v.vertexCoords);
        v2f.normal = Interpolator::viewspace_interpolate(bar, a2v.normals);
        v2f.textureCoord = Interpolator::viewspace_interpolate(bar, a2v.textureCoords);
    }

    // Phong shading with texture
    virtual void shadeFragment(ColorN& color) override {
        Point3 reflectPoint = v2f.fragCoord;
        Point2 uv = v2f.textureCoord;
        shared_ptr<BlinnPhongWithBump> material1 = *reinterpret_cast<shared_ptr<BlinnPhongWithBump>*>(&uni.material);
        shared_ptr<Texture> bumpMap = material1->bump;
        double kh = 0.2, kn = 0.1;
        //TBN
        auto normalTexture = [&](double u, double v) {
            //RGB值作为向量的长度即为高度
            double du = 1.0 / bumpMap->width();
            double dv = 1.0 / bumpMap->height();
            double h = bumpMap->valueRGB255(u, v).norm();
            double dh_u = kh * kn * (bumpMap->valueRGB255(u + du, v).norm() - h);
            double dh_v = kh * kn * (bumpMap->valueRGB255(u, v + dv).norm() - h);
            return Vec3(-dh_u, -dh_v, 1);
        };

        Vec3 normalTS = normalTexture(uv[0], uv[1]);
        Vec3 normalWS = (v2f_common.tbn * normalTS).normalized();

        shared_ptr<BlinnPhong> material = *reinterpret_cast<shared_ptr<BlinnPhong>*>(&uni.material);
        TextureShader::BlinnPhongPayload payload = { reflectPoint,normalWS,uv,material,uni.ambientLightIntensity,uni.lights };
        //NormalShader::drawNormal(normalWS, color);
        blinnPhong(payload, color);
    }

};

#endif