#pragma once
#ifndef HITTABLE_TRIANGLE_H
#define HITTABLE_TRIANGLE_H
#include <hittable/Hittable.h>


class Triangle : public Hittable {
public:
	virtual bool hit(const Ray& r, Interval ray_t, HitRecord& rec) const override{
		return false;
	}

	virtual void set_face_normal(const Ray& r, const Vec3& outward_normal, HitRecord& rec) const  override {

	}




	Vec4 vertices[3];
	Vec3 vertices_vec3[3];
	ColorN colors[3];
	Point2 texCoords[3]; //贴图uv
	Vec4 normals[3];
	shared_ptr<Material> material;
	Vec3 tangents[3];	//切向量(构成TBN空间)
};


#endif
