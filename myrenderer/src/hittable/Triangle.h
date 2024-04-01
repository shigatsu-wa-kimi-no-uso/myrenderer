#pragma once
#ifndef TRIANGLE_H
#define TRIANGLE_H
#include <hittable/Hittable.h>


class Triangle : public Hittable {
public:
	virtual bool hit(const Ray& r, Interval ray_t, HitRecord& rec) const {
		return false;
	}

	virtual void set_face_normal(const Ray& r, const Vec3& outward_normal, HitRecord& rec) const {

	}

	Vec4 vertices[3];
	Vec3 vertices_vec3[3];
	ColorN colors[3];
	Vec2 texCoords[3]; //贴图uv
	Vec4 normals[3];
	Vec3 normals_vec3[3]; //同步更新
	shared_ptr<Material> material;

};


#endif
