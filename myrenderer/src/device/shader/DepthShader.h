#pragma once
#ifndef DEVICE_SHADER_DEPTH_SHADER_H
#define DEVICE_SHADER_DEPTH_SHADER_H
#include <device/shader/Shader.h>

class DepthShader : public Shader {
public:

	static bool isLit(const Light& l, const Mat4& transShadowMap, const Mat4& transLightViewspace, const Point3& viewspaceCoord,const Mat4& viewInverse, double epsilon) {
		Vec4 worldSpaceCoord = viewInverse * toVec4(viewspaceCoord, 1);
		Point3 shadowMapCoord = toVec3(transShadowMap * worldSpaceCoord);

		//特别注意, 要保证光源pos和渲染点pos所在的空间一致,且保证空间未拉伸
		//即保证在计算是否照亮时的distance值与在depth shader中计算distance时的空间距离的单位长度相同, 光源pos未经变换而是在外部直接设置,因此处于世界坐标系
		double distance = -(transLightViewspace * worldSpaceCoord).z() / l.maxDistance;   //平行光 只使用z轴深度 (viewspace下) z一定小于零故取反
		int x = round(shadowMapCoord.x());
		int y = round(shadowMapCoord.y());
		const ShadowMap& map = l.shadowMap;
		if (x >= 0 && x < map.width && y >= 0 && y < map.height) {
			double firstHitDist = (*map.buffer)[y][x][0][0];
			//std::clog << firstHitDist << " " << distance << "\n";
			if (firstHitDist < 0.01) {
				return true;
			}
			if (distance < firstHitDist + epsilon){// && distance > firstHitDist - epsilon) {
				return true;
			}

		}
		return false;
	}


	//shadow map实现硬阴影
	virtual void shadeVertex(int nthVertex, Vec4& clip_coord) override {
		Vec4 vertex_view_coord = uni.modelView * toVec4(attr.vertexCoord, 1);
		interpolator.vertex_view_coords[nthVertex] = toVec3(vertex_view_coord);
		a2v.vertexCoords[nthVertex] = interpolator.vertex_view_coords[nthVertex];
		clip_coord = uni.projection * vertex_view_coord;
	}

	virtual void processVarying(const Vec3& bar) override {
		v2f.fragCoord = interpolator.viewspace_interpolate(bar, a2v.vertexCoords);
	}

	virtual void shadeFragment(ColorN& color) override {
		//颜色是viewspace下该点到"摄像头"的距离,即深度 实现平行光引用,故用z轴坐标代替距离
		double depth = -v2f.fragCoord.z();  // 平行光 只记录z(在光源的viewspace下)(摄像头朝向-z轴,故z永远小于0,取正值便于绘制shadow map)
		depth /= uni.maxDistance;  //[0,maxDistance] -> [0,1]
		color = depth*ColorN(1,1,1);
	}

	virtual shared_ptr<Shader> clone() const override {
		return make_shared<DepthShader>(*this);
	}
};

#endif // DEVICE_SHADER_NORMAL_SHADER_H
