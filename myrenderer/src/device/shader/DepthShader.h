#pragma once
#ifndef DEVICE_SHADER_DEPTH_SHADER_H
#define DEVICE_SHADER_DEPTH_SHADER_H
#include <device/shader/Shader.h>

class DepthShader : public Shader {
public:

	static bool isLit(const Light& l, const Mat4& transShadowMap, const Mat4& transLightViewspace, const Point3& viewspaceCoord,const Mat4& viewInverse, double epsilon) {
		Vec4 worldSpaceCoord = viewInverse * toVec4(viewspaceCoord, 1);
		Point3 shadowMapCoord = toVec3(transShadowMap * worldSpaceCoord);

		//�ر�ע��, Ҫ��֤��Դpos����Ⱦ��pos���ڵĿռ�һ��,�ұ�֤�ռ�δ����
		//����֤�ڼ����Ƿ�����ʱ��distanceֵ����depth shader�м���distanceʱ�Ŀռ����ĵ�λ������ͬ, ��Դposδ���任�������ⲿֱ������,��˴�����������ϵ
		double distance = -(transLightViewspace * worldSpaceCoord).z() / l.maxDistance;   //ƽ�й� ֻʹ��z����� (viewspace��) zһ��С�����ȡ��
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


	//shadow mapʵ��Ӳ��Ӱ
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
		//��ɫ��viewspace�¸õ㵽"����ͷ"�ľ���,����� ʵ��ƽ�й�����,����z������������
		double depth = -v2f.fragCoord.z();  // ƽ�й� ֻ��¼z(�ڹ�Դ��viewspace��)(����ͷ����-z��,��z��ԶС��0,ȡ��ֵ���ڻ���shadow map)
		depth /= uni.maxDistance;  //[0,maxDistance] -> [0,1]
		color = depth*ColorN(1,1,1);
	}

	virtual shared_ptr<Shader> clone() const override {
		return make_shared<DepthShader>(*this);
	}
};

#endif // DEVICE_SHADER_NORMAL_SHADER_H
