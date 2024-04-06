#pragma once
#ifndef DEVICE_SHADER_CHECKER_TEXTURE_SHADER_H
#define DEVICE_SHADER_CHECKER_TEXTURE_SHADER_H
#include <device/shader/Shader.h>
#include <material/Lambertian.h>
#include <device/shader/TextureShader.h>

class CheckerTextureShader : public Shader {

    virtual void shadeVertex(int nthVertex, Vec4& clip_coord) override {
        Vec4 vertex_view_coord = uni.modelView * toVec4(attr.vertexCoord, 1);
        Vec3 normal = toVec3(uni.modelViewIT * toVec4(attr.normal, 0));//�任������Ҫ���з���������
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
        Point3 worldPos = toVec3(uni.viewInverse*toVec4(v2f.fragCoord,1)); //��world�ռ�����Ϊ׼����viewspace�»����ӽǵĸı���ı�
        shared_ptr<BlinnPhong> material = *reinterpret_cast<shared_ptr<BlinnPhong>*>(&uni.material);

        int x = floor(worldPos[0]*2 + epsilon); //�ж�������[0,1]����[1,2]����1,��[1-epsilon,1+epsilon]��Χ��Ϊ���µ�,����ֱ����Ϊ��1
        int y = floor(worldPos[1]*2 + epsilon); //����һ��������һ��Ϊ-1�ı��ʽ,������ֵ���,���½��Ϊ��-1.000001��ֵ
        int z = floor(worldPos[2]*2 + epsilon); //��ʱ��Ȼ�����������Ϊ-1,������floor����,floor(-1)==-1,floor(-1.000001) == -2,�Ӷ��������ͼ����ʧ��
        int u = x + y + z; //��ֹxyzȫ����ת��Ϊdouble����,������
        /*
        int x2 = floor(worldPos[0] * 2);
        int y2 = floor(worldPos[1] * 2);
        int z2 = floor(worldPos[2] * 2);
        std::cout << worldPos[0] * 2 << " " << worldPos[1] * 2 << " " << worldPos[2] * 2 <<" "<< x2 + y2 + z2 << "\n";
        std::cout << worldPos[0] * 2 + epsilon << " " << worldPos[1] * 2 + epsilon << " " << worldPos[2] * 2 + epsilon << " " << u << "\n";*/
        Point2 uv(u, 0);
        Vec3 normal = v2f.normal.normalized();

        //��Ը�shader,����ʵ�ʲ�����ʲô,��ת��ΪBlinnPhong����
        //���Բ���-shader֮��Ӧ������ȷ�İ󶨹�ϵ,��������
        //ģ��-����-��ͼ-shader֮���Ϊ��Զ��ϵ,��ֻҪ����ת������Ԥ�ڼ��������

        TextureShader::BlinnPhongPayload payload = { v2f.fragCoord,normal,uv,material,uni.ambientLightIntensity,uni.lights,uni.objWorld2ShadowMap,uni.objWorld2LightViewspace,uni.viewInverse,uni.withShadow };

        TextureShader::blinnPhong(payload, color);
    }

};

#endif