#pragma once
#ifndef COMMON_UTILITY_H
#define COMMON_UTILITY_H
#include <common/global.h>

inline double deg2rad(double deg) {
	return deg * pi / 180.0;
}

Vec3 toVec3(const Vec4& v) {
	return Vec3(v.x(), v.y(), v.z());
}

Vec4 toVec4(const Vec3& v, double w) {
	return Vec4(v.x(), v.y(), v.z(), w);
}

inline time_t current_time_nanos() {
	using namespace std;
	chrono::system_clock::time_point now = chrono::system_clock::now();
	chrono::nanoseconds nanoseconds = chrono::duration_cast<chrono::nanoseconds>(now.time_since_epoch());
	return nanoseconds.count();
}

inline double random_double() {
	// Returns a random real in [0,1).
	using namespace std;
	static default_random_engine dre((uint32_t)current_time_nanos());
	static uniform_real_distribution<double> uniform(0, 1);
	return uniform(dre);
}

inline double random_double(double min, double max) {
	// Returns a random real in [min,max).
	return min + (max - min) * random_double();
}

inline Vec3 random_unit_vector() {
	return Vec3::Random().normalized();
}

inline Vec3 random_in_unit_disk() {
	while (true) {
		auto p = Vec3(random_double(-1.0, 1.0), random_double(-1.0, 1.0), 0);
		if (p.squaredNorm() < 1)
			return p;
	}
}


inline Vec3 random_on_hemisphere(const Vec3& normal) {
	Vec3 on_unit_sphere = random_unit_vector();
	if (on_unit_sphere.dot(normal) > 0.0) { // In the same hemisphere as the normal
		return on_unit_sphere;
	} else {
		return -on_unit_sphere;
	}
}



inline bool vec_near_zero(const Vec3& v) {
	// Return true if the vector is close to zero in all dimensions.
	double s = 1e-8;
	return (fabs(v(0)) < s) && (fabs(v(1)) < s) && (fabs(v(2)) < s);
}


static inline Mat4 getRotation(Vec3 axis, double angle)
{
	Mat3 rotation;
	Mat4 r4;
	axis.normalize();
	double nx = axis.x(), ny = axis.y(), nz = axis.z();
	double cosa = std::cos(pi * angle / 180), sina = std::sin(pi * angle / 180);
	rotation <<
		0, -nz, ny,
		nz, 0, -nx,
		-ny, nx, 0;
	rotation *= sina;
	rotation += cosa * Mat3::Identity();
	rotation += (1 - cosa) * axis * axis.transpose();
	r4.block(0, 0, 3, 3) << rotation;
	r4.col(3) << 0, 0, 0, 1;
	r4.row(3).head(3) << 0, 0, 0;
	return r4;
}

inline Mat4 getModeling(const Point3& world_coord, const Vec3& scales, const Vec3& rotation_axis, double rotation_angle) {
	Mat4 rotation = getRotation(rotation_axis, rotation_angle);
	Mat4 scale;
	scale << 
		scales.x(), 0, 0, 0,
		0, scales.y(), 0, 0,
		0, 0, scales.z(), 0,
		0, 0, 0, 1;

	Mat4 translate;
	translate << 1, 0, 0, world_coord.x(),
		0, 1, 0, world_coord.y(),
		0, 0, 1, world_coord.z(),
		0, 0, 0, 1;

	return translate * rotation * scale;

}
#endif // COMMON_UTILITY_H