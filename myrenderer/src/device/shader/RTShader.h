#pragma once
#ifndef DEVICE_SHADER_RTSHADER_H
#define DEVICE_SHADER_RTSHADER_H
#include <device/shader/Shader.h>

class RTShader : public Shader {
public:
	virtual void shadeVertex(int nthVertex, Vec4& clip_coord) override {
		Vec4 vertex_view_coord = uni.modelView * toVec4(attr.vertexCoord, 1);
		a2v.normals[nthVertex] = toVec3(uni.modelViewIT * toVec4(attr.normal, 0));
		interpolator.vertex_view_coords[nthVertex] = toVec3(vertex_view_coord);
		clip_coord = uni.projection * vertex_view_coord;
	}

	virtual void processVarying(const Vec3& bar) override {
		v2f.fragCoord = interpolator.viewspace_interpolate(bar, a2v.vertexCoords);
		v2f.normal = interpolator.viewspace_interpolate(bar, a2v.normals);
	}

	virtual void shadeFragment(ColorN& color) override {
		Vec3 rayDirection = v2f.fragCoord.normalized();
		HitRecord rec;
		rec.hitAnything = true;
		rec.frontFace = v2f.normal.dot(rayDirection) < 0 ? true : false;
		rec.normal = v2f.normal;
		rec.point = v2f.fragCoord;
		rec.material = uni.material;
		color = uni.rayTracer->shade(rec , -rayDirection);
	}

	virtual shared_ptr<Shader> clone() const override {
		return make_shared<RTShader>(*this);
	}
};

#endif // !DEVICE_SHADER_RTSHADER_H
