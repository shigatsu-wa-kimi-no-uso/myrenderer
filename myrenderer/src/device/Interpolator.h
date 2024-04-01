#pragma once
#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H
#include <common/global.h>

class Interpolator {
public:
	static Vec3 vertex_view_coords[3]; //静态,只有这一个实例,并发情况下需改成多Interpolator非静态变量,或静态变量列表

	static Vec3 computeBarycentric2D(const Point2& p, const Vec4(&v)[3]) {
		double x = p.x(), y = p.y();
		double c1 = (x * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * y + v[1].x() * v[2].y() - v[2].x() * v[1].y()) / (v[0].x() * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * v[0].y() + v[1].x() * v[2].y() - v[2].x() * v[1].y());
		double c2 = (x * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * y + v[2].x() * v[0].y() - v[0].x() * v[2].y()) / (v[1].x() * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * v[1].y() + v[2].x() * v[0].y() - v[0].x() * v[2].y());
		double c3 = (x * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * y + v[0].x() * v[1].y() - v[1].x() * v[0].y()) / (v[2].x() * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * v[2].y() + v[0].x() * v[1].y() - v[1].x() * v[0].y());
		return Vec3(c1, c2, c3);
	}

	template<class T>
	static T screenspace_interpolate(const Vec3& bar, const T(&v)[3]) {
		return bar.x() * v[0] + bar.y() * v[1] + bar.z() * v[2];
	}
	
	template<class T>
	static T viewspace_interpolate(const Vec3& bar, const T(&v)[3]) {
		Vec3 view_vz = { vertex_view_coords[0].z(), vertex_view_coords[1].z() ,vertex_view_coords[2].z() };
		double view_z = 1.0 / (bar.x() / view_vz[0] + bar.y() / view_vz[1] + bar.z() / view_vz[2]);
		T view_attrib = bar.x() * v[0] / view_vz[0] + bar.y() * v[1] / view_vz[1] + bar.z() * v[2] / view_vz[2];
		return view_attrib * view_z;
	}
};

inline Vec3 Interpolator::vertex_view_coords[3];


#endif // INTERPOLATOR_H
