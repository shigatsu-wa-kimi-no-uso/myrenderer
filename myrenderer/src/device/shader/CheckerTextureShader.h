#pragma once
#ifndef DEVICE_SHADER_CHECKER_TEXTURE_SHADER_H
#define DEVICE_SHADER_CHECKER_TEXTURE_SHADER_H
#include <device/shader/Shader.h>
#include <material/Lambertian.h>
#include <device/shader/TextureShader.h>

class CheckerTextureShader : public Shader {

    virtual void shadeVertex(int nthVertex, Vec4& clip_coord) override {
        Vec4 vertex_view_coord = uni.modelView * toVec4(attr.vertexCoord, 1);
        Vec3 normal = toVec3(uni.modelViewIT * toVec4(attr.normal, 0));//变换过程中要进行法向量矫正
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


    virtual void shadeFragment(ColorN& color) override {
        double epsilon = 0.001;
        Point3 worldPos = toVec3(uni.viewInverse*toVec4(v2f.fragCoord,1)); //以world空间坐标为准！在viewspace下会因视角的改变而改变
        shared_ptr<BlinnPhong> material = *reinterpret_cast<shared_ptr<BlinnPhong>*>(&uni.material);

        int x = floor(worldPos[0]*2 + epsilon); //判断数落在[0,1]还是[1,2]还是1,在[1-epsilon,1+epsilon]范围内为误差导致的,可以直接认为是1
        int y = floor(worldPos[1]*2 + epsilon); //对于一个计算结果一定为-1的表达式,由于数值误差,导致结果为如-1.000001等值
        int z = floor(worldPos[2]*2 + epsilon); //此时虽然表面上输出仍为-1,但对于floor函数,floor(-1)==-1,floor(-1.000001) == -2,从而导致输出图像有失真
        int u = x + y + z; //防止xyz全部被转换为double计算,造成误差
        /*
        int x2 = floor(worldPos[0] * 2);
        int y2 = floor(worldPos[1] * 2);
        int z2 = floor(worldPos[2] * 2);
        std::cout << worldPos[0] * 2 << " " << worldPos[1] * 2 << " " << worldPos[2] * 2 <<" "<< x2 + y2 + z2 << "\n";
        std::cout << worldPos[0] * 2 + epsilon << " " << worldPos[1] * 2 + epsilon << " " << worldPos[2] * 2 + epsilon << " " << u << "\n";*/
        Point2 uv(u, 0);
        Vec3 normal = v2f.normal.normalized();

        //针对该shader,无论实际材质是什么,均转换为BlinnPhong材质
        //所以材质-shader之间应当有正确的绑定关系,否则会出错
        //模型-材质-贴图-shader之间均为多对多关系,但只要类型转换符合预期即不会出错

        TextureShader::BlinnPhongPayload payload = { v2f.fragCoord,normal,uv,material,uni.ambientLightIntensity,uni.lights,uni.objWorld2ShadowMap,uni.objWorld2LightViewspace,uni.viewInverse,uni.withShadow };

        TextureShader::blinnPhong(payload, color);
    }

};

#endif