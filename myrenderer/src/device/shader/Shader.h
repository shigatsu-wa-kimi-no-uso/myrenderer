#pragma once
#ifndef SHADER_H
#define SHADER_H
#include <common/global.h>
#include <common/utility.h>
#include <device/Interpolator.h>
#include <unordered_map>

class Shader {
public:
	struct Uniform {
		Mat4 model;
		Mat4 view;
		Mat4 projection;
	}uni;

	struct Attribute {
		Vec3 vertexCoord;
		Vec3 normal;
	}attr;

	virtual void processVarying(const Vec3& bar) = 0;

	virtual void shadeVertex(int nthVertex, Vec4& clip_coord) {
		Mat4 mv = uni.view * uni.model;
		Vec4 vertex_view_coord = mv * toVec4(attr.vertexCoord, 1);
		Interpolator::vertex_view_coords[nthVertex] = vertex_view_coord.head(3);
		a2v.vertexCoords[nthVertex] = attr.vertexCoord;
		clip_coord = uni.projection * vertex_view_coord;
	}

	virtual void shadeFragment(ColorN& color) = 0;

protected:
	struct VertexVarying {
		Vec3 normals[3];
		Vec3 vertexCoords[3];
	}a2v;

	struct FragmentVarying {
		Vec3 fragCoord;
		Vec3 normal;
	}v2f;
};


#endif // SHADER_H
