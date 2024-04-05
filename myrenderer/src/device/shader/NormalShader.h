#pragma once
#ifndef DEVICE_SHADER_NORMAL_SHADER_H
#define DEVICE_SHADER_NORMAL_SHADER_H
#include <device/shader/Shader.h>
#include <material/BlinnPhong.h>

class NormalShader : public Shader{
public:
	virtual void shadeVertex(int nthVertex, Vec4& clip_coord) override{
		Mat4 mv = uni.view * uni.model;
		Vec4 vertex_view_coord = mv * toVec4(attr.vertexCoord, 1);
		Mat4 mvIT = mv.inverse().transpose();  //变换过程中要进行法向量矫正！
		a2v.normals[nthVertex] = toVec3(mvIT * toVec4(attr.normal,0));
		Interpolator::vertex_view_coords[nthVertex] = toVec3(vertex_view_coord);
		clip_coord = uni.projection * vertex_view_coord;
	}

	virtual void processVarying(const Vec3& bar) override{
		v2f.fragCoord = Interpolator::viewspace_interpolate(bar,a2v.vertexCoords);
		v2f.normal = Interpolator::viewspace_interpolate(bar, a2v.normals);
	}

	static void drawNormal(const Vec3& normal, ColorN& color) {
		color = (normal.normalized() + Vec3(1, 1, 1)) / 2.0;
	}

	virtual void shadeFragment(ColorN& color) override {
		drawNormal(v2f.normal, color);
	}
};

#endif // DEVICE_SHADER_NORMAL_SHADER_H
