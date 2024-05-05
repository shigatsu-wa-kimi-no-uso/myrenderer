#pragma once
#ifndef DEVICE_SHADER_TEXTURE_SHADER_H
#define DEVICE_SHADER_TEXTURE_SHADER_H
#include <device/shader/Shader.h>
#include <material/BlinnPhong.h>

class TextureShader : public Shader {
public:
	virtual void shadeVertex(int nthVertex, Vec4& clip_coord) override {
        Vec4 vertex_view_coord = uni.modelView * toVec4(attr.vertexCoord, 1);
		a2v.normals[nthVertex] = toVec3(uni.modelViewIT * toVec4(attr.normal, 0));
        a2v.textureCoords[nthVertex] = attr.textureCoord;
        interpolator.vertex_view_coords[nthVertex] = toVec3(vertex_view_coord);
        a2v.vertexCoords[nthVertex] = interpolator.vertex_view_coords[nthVertex];
		clip_coord = uni.projection * vertex_view_coord;
	}

	virtual void processVarying(const Vec3& bar) override {
		v2f.fragCoord = interpolator.viewspace_interpolate(bar, a2v.vertexCoords);
		v2f.normal = interpolator.viewspace_interpolate(bar, a2v.normals);
        v2f.textureCoord = interpolator.viewspace_interpolate(bar, a2v.textureCoords);
	}

    struct BlinnPhongPayload {
        const Point3& fragCoord;
        const Vec3& normal;
        const Point2& textureCoord;
        const shared_ptr<BlinnPhong>& material;
        const Vec3& ambientLightIntensity;
        const std::vector<Light>& lights;
        const std::vector<Mat4>& objWorld2ShadowMap;
        const std::vector<Mat4>& objWorld2LightViewspace;
        const Mat4& viewInverse;
        bool withShadow;
    };

    static void blinnPhong(const BlinnPhongPayload& payload,ColorN& color) {
        Point3 reflectPoint = payload.fragCoord;
        Vec3 normal = payload.normal.normalized();
        Point2 uv = payload.textureCoord;
        //针对该shader,无论实际材质是什么,均转换为BlinnPhong材质
        //所以材质-shader之间应当有正确的绑定关系,否则会出错
        //模型-材质-贴图-shader之间均为多对多关系,但只要类型转换符合预期即不会出错
        const shared_ptr<BlinnPhong>& material = payload.material;
        shared_ptr<Texture> texture = material->diffuse;
        ColorN ka = material->ambient;
        ColorN kd = material->diffuse->value(uv[0], uv[1]);
        ColorN ks = material->specular;
        double p = material->p;
        /* std::cout << "u = " << uv[0] << " v = " << uv[1] << "\n";
         std::cout << kd[0] << " " << kd[1] << " " << kd[2] << "\n";*/
        Vec3 ambientLightIntensity = payload.ambientLightIntensity;

        Vec3 eye_pos = Vec3(0, 0, 0); //给定眼睛位置,但在view space下, 眼睛位置应为摄像头位置,固定在原点

        ColorN result_color(0, 0, 0);
        Vec3 view_vec = (eye_pos - reflectPoint).normalized();
        ColorN i_amb = ka.cwiseProduct(ambientLightIntensity);
        // ambient(env) + diffuse(Lambert) + specular(Phong)
        // k in [0,1]
        // I = Iamb + Idiff + Ispec = Ka*Ia + Kd*(I/r^2)*max(0,cos<normal,light>) + Ks*(I/r^2)*max(0,cos<normal,half>)^p

        result_color += i_amb;
        const std::vector<Light>& lights = payload.lights; 
        const std::vector<Mat4>& transShadowMap = payload.objWorld2ShadowMap;
        const std::vector<Mat4>& transLightViewspace = payload.objWorld2LightViewspace;
        int lightCnt = lights.size();
        for (int i = 0; i < lightCnt; i++)
        {
            const Light& l = lights[i];
            //阴影判断
            if (payload.withShadow && !DepthShader::isLit(l, transShadowMap[i],transLightViewspace[i], payload.fragCoord,payload.viewInverse, 0.08)) {
                continue;
            }

            double r2 = (l.position - reflectPoint).squaredNorm();   // r^2
            Vec3 intensityArrived = l.intensity / r2;   // I/r^2
            Vec3 l_vec = (l.position - reflectPoint).normalized();  //入射光线单位向量反方向
            Vec3 half_vec = (view_vec + l_vec).normalized();
            double cos_n_l = normal.dot(l_vec);
            double cos_n_h = normal.dot(half_vec);
            ColorN i_diff = kd.cwiseProduct(intensityArrived) * std::max(0.0, cos_n_l);
            ColorN i_spec = ks.cwiseProduct(intensityArrived) * std::pow(std::max(0.0, cos_n_h), p);
            result_color += i_diff + i_spec;
        }
        color = result_color;

    }

    // Phong shading with texture
	virtual void shadeFragment(ColorN& color) override {
        Point3 reflectPoint = v2f.fragCoord;
        Vec3 normal = v2f.normal.normalized();
        Point2 uv = v2f.textureCoord;
  
        //针对该shader,无论实际材质是什么,均转换为BlinnPhong材质
        //所以材质-shader之间应当有正确的绑定关系,否则会出错
        //模型-材质-贴图-shader之间均为多对多关系,但只要类型转换符合预期即不会出错
        shared_ptr<BlinnPhong> material = *reinterpret_cast<shared_ptr<BlinnPhong>*>(&uni.material);

        BlinnPhongPayload payload = { reflectPoint,normal,uv,material,uni.ambientLightIntensity,uni.lights ,uni.objWorld2ShadowMap,uni.objWorld2LightViewspace,uni.viewInverse,uni.withShadow};

        blinnPhong(payload, color);
	}

    virtual shared_ptr<Shader> clone() const override {
        return make_shared<TextureShader>(*this);
    }
};

#endif // DEVICE_SHADER_TEXTURE_SHADER_H