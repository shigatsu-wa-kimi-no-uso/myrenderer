#pragma once
#ifndef DEVICE_SHADER_DEPTH_SHADER_H
#define DEVICE_SHADER_DEPTH_SHADER_H
#include <device/shader/Shader.h>

class DepthShader : public Shader {
public:
	//shadow map µœ÷”≤“ı”∞
	virtual void shadeVertex(int nthVertex, Vec4& clip_coord) override {
		Mat4 mv = uni.view * uni.model;
		Vec4 vertex_view_coord = mv * toVec4(attr.vertexCoord, 1);
		Interpolator::vertex_view_coords[nthVertex] = toVec3(vertex_view_coord);
		clip_coord = uni.projection * vertex_view_coord;
	}

	virtual void processVarying(const Vec3& bar) override {
		v2f.fragCoord = Interpolator::viewspace_interpolate(bar, a2v.vertexCoords);
	}

	virtual void shadeFragment(ColorN& color) override {
		color = ColorN(1, 1, 1);
	}
};

#endif // DEVICE_SHADER_NORMAL_SHADER_H
