#pragma once
#ifndef DEVICE_SHADER_SHADER_H
#define DEVICE_SHADER_SHADER_H
#include <common/global.h>
#include <common/utility.h>
#include <common/Light.h>
#include <device/Interpolator.h>
#include <unordered_map>
#include <texture/Texture.h>
#include <material/Material.h>
#include <common/Buffer.h>

class Shader {
public:
	struct Uniform {
		Mat4 modelView;
		Mat4 modelViewIT;
		Mat4 projection;
		Mat4 viewInverse;
		shared_ptr<Material> material;
		std::vector<Light> lights;
		std::vector<Mat4> objWorld2ShadowMap;
		std::vector<Mat4> objWorld2LightViewspace;
		shared_ptr<std::vector<ShadowMap>> shadowMaps;
		Vec3 ambientLightIntensity;
		double maxDistance;
		bool withShadow;
	}uni;

	struct Attribute {
		Vec3 vertexCoord;
		Vec2 textureCoord;
		Vec3 normal;
		Vec3 tangent;
	}attr;


	virtual void processVarying(const Vec3& bar) = 0;

	virtual void shadeVertex(int nthVertex, Vec4& clip_coord) {
		Vec4 vertex_view_coord = uni.modelView * toVec4(attr.vertexCoord, 1);
		Interpolator::vertex_view_coords[nthVertex] = vertex_view_coord.head(3);
		a2v.vertexCoords[nthVertex] = attr.vertexCoord;
		clip_coord = uni.projection * vertex_view_coord;
		
	}

	virtual void shadeFragment(ColorN& color) = 0;
	struct FragmentVarying {
		Vec3 fragCoord;
		Vec3 normal;
		Vec2 textureCoord;
	}v2f;



protected:
	struct V2F_Vars {
		Mat3 tbn;
	}v2f_common;

	struct VertexVarying {
		Vec3 normals[3];
		Vec3 vertexCoords[3];
		Vec2 textureCoords[3];
	}a2v;

};


#endif // DEVICE_SHADER_SHADER_H
