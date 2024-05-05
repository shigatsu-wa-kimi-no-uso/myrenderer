#pragma once
#ifndef DEVICE_SHADER_NORMAL_SHADER_H
#define DEVICE_SHADER_NORMAL_SHADER_H
#include <device/shader/Shader.h>
#include <material/BlinnPhong.h>

class NormalShader : public Shader{
public:
	virtual void shadeVertex(int nthVertex, Vec4& clip_coord) override{
		Vec4 vertex_view_coord = uni.modelView * toVec4(attr.vertexCoord, 1);
		a2v.normals[nthVertex] = toVec3(uni.modelViewIT * toVec4(attr.normal,0));
		interpolator.vertex_view_coords[nthVertex] = toVec3(vertex_view_coord);
		clip_coord = uni.projection * vertex_view_coord;
	}

	virtual void processVarying(const Vec3& bar) override{
		v2f.fragCoord = interpolator.viewspace_interpolate(bar,a2v.vertexCoords);
		v2f.normal = interpolator.viewspace_interpolate(bar, a2v.normals);
	}

	static void drawNormal(const Vec3& normal, ColorN& color) {
		color = (normal.normalized() + Vec3(1, 1, 1)) / 2.0;
	}

	virtual void shadeFragment(ColorN& color) override {
		drawNormal(v2f.normal, color);
	}

	virtual shared_ptr<Shader> clone() const override {
		return make_shared<NormalShader>(*this);
	}
};

#endif // DEVICE_SHADER_NORMAL_SHADER_H
