#pragma once
#ifndef DEVICE_SHADER_CHECKER_TEXTURE_SHADER_H
#define DEVICE_SHADER_CHECKER_TEXTURE_SHADER_H
#include <device/shader/Shader.h>
#include <material/Lambertian.h>

class CheckerTextureShader : public Shader {

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
    }

    virtual void processVarying(const Vec3& bar) override {
        v2f.fragCoord = Interpolator::viewspace_interpolate(bar, a2v.vertexCoords);
        v2f.normal = Interpolator::viewspace_interpolate(bar, a2v.normals);
    }

    // Phong shading with texture
    virtual void shadeFragment(ColorN& color) override {
        Point3 pos = v2f.fragCoord;
        shared_ptr<Lambertian> material = *reinterpret_cast<shared_ptr<Lambertian>*>(&uni.material);
        shared_ptr<Texture> checkerTexture = material->diffuse;
        int x = floor(pos[0]*2); 
        int y = floor(pos[1]*2);
        int z = floor(pos[2]*2);
        int u = x + y + z; //防止xyz全部被转换为double计算,造成误差
       
        color = checkerTexture->value(u , 0);
    }

};

#endif