#pragma once
#ifndef COMMON_UTILITY_H
#define COMMON_UTILITY_H
#include <common/global.h>
#include <iostream>
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

inline Vec2 random_in_unit_circle() {
	while (true) {
		Vec2 p(random_double(-1.0, 1.0), random_double(-1.0, 1.0));
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



inline Mat4 getViewing(const Point3& eyePos,const Vec3& lookat,const Vec3& vup) {
	Mat4 translate;
	translate << 1, 0, 0, -eyePos[0],
		0, 1, 0, -eyePos[1],
		0, 0, 1, -eyePos[2],
		0, 0, 0, 1;
	Vec3 g = lookat;
	Vec3 t = vup;
	Vec3 gxt = t.cross(-g);
	//std::clog << g.dot(t) << " " << t.dot(gxt) << " " << gxt.dot(g) << "\n";
	Mat4 view;
	view << gxt[0], gxt[1], gxt[2], 0,
		t[0], t[1], t[2], 0,
		-g[0], -g[1], -g[2], 0,
		0, 0, 0, 1;
	view = translate * view;
	return view;
}


inline Mat4 getOrthoProjection(double f,double n,double l,double r,double t,double b) {
	Mat4 ortho, trans, scale;
	trans <<
		1, 0, 0, -(r + l) / 2,   // [l,r] -> [-(r-l)/2,(r-l)/2]
		0, 1, 0, -(t + b) / 2,  
		0, 0, 1, -(n + f) / 2,   // [n,f]/[f,n] -> [-abs(f-n)/2,abs(f-n)/2] (n f的相对位置不变,若n<f,则平移后小的仍为n,大的仍为f)
		0, 0, 0, 1;
	scale <<							
		2 / abs(r - l), 0, 0, 0,	// [-(r-l)/2,(r-l)/2] -> [-1,1]
		0, 2 / abs(t - b), 0, 0,
		0, 0, 2 / abs(n - f), 0,	// [-abs(f-n)/2,abs(f-n)/2] -> [-1,1]  //相对位置不变
		0, 0, 0, 1;

	ortho = scale * trans;
	return ortho;
}


inline Mat4 getOrthoProjection(double f, double n, double width,double aspectRatio) {
	double r = width / 2.0;
	double t = r / aspectRatio;
	double b = -t, l = -r;
	Mat4 ortho, trans, scale;
	trans <<
		1, 0, 0, -(r + l) / 2,   // [l,r] -> [-(r-l)/2,(r-l)/2]
		0, 1, 0, -(t + b) / 2,
		0, 0, 1, -(n + f) / 2,   // [n,f]/[f,n] -> [-abs(f-n)/2,abs(f-n)/2] (n f的相对位置不变,若n<f,则平移后小的仍为n,大的仍为f)
		0, 0, 0, 1;
	scale <<
		2 / abs(r - l), 0, 0, 0,	// [-(r-l)/2,(r-l)/2] -> [-1,1]
		0, 2 / abs(t - b), 0, 0,
		0, 0, 2 / abs(n - f), 0,	// [-abs(f-n)/2,abs(f-n)/2] -> [-1,1]  //相对位置不变
		0, 0, 0, 1;
	ortho = scale * trans;
	return ortho;
}


inline Mat4 getProjection(double frustumFar,double frustumNear,double degFov,double aspectRatio) {
	// 远近与[-1,1]的关系: zFar --> -1 zNear --> 1 
	//默认从(0,0,0)看向(0,0,-1)的视野
	Mat4 projection;
	double f = frustumFar, n = frustumNear;			// 定义 camera 在camera坐标系下面向 -z 轴, 越远越小
	double t = std::tan(pi * (degFov / 2.0) / 180.0) * std::abs(n);
	double b = -t, r = t * aspectRatio, l = -r;  //基于世界坐标系的b,t,l,r  与canvas的大小即输出图像的大小不是一回事
	// camera的aspect_ratio与输出图像的aspect ratio可能不相同
	Mat4 ortho, persp2ortho;
	ortho = getOrthoProjection(f, n, l, r, t, b);
	persp2ortho <<
		n, 0, 0, 0,
		0, n, 0, 0,
		0, 0, n + f, -(n * f),
		0, 0, 1, 0;
	// 视锥体->裁剪空间			
	//视野->透视转正交->标准正方体(裁剪)->透视除法(外部)->NDC
	projection = ortho * persp2ortho;
	return projection;
}

inline Mat4 getViewport(int width,int height,int zFar,int zNear) {
	//注意: 如果对z轴操作,则可能改变z轴值的大小符号与远近的关系
	//此变换中, ndc_z = -1 --> screen_z=zNear, ndc_z = 1 --> screen_z=zFar,  zFar < zNear < 0
	//输入: NDC([-1,1]^3)   near = -1, far = 1
	double f1 = abs(zFar - zNear) / 2.0; // zLen/2.0
	double f2 = abs(zFar + zNear) / 2.0; // 有绝对值！
	Mat4 t, s, t2;
	//[-1,1] -->[0,2]
	t <<
		1, 0, 0, 1,
		0, 1, 0, 1,
		0, 0, 1, 0,
		0, 0, 0, 1;
	s <<
		0.5 * width, 0, 0, 0,	//[0,2]^2->[0,width]*[0,height]
		0, 0.5 * height, 0, 0,
		0, 0, f1, 0,			//[-1(far),1(near)] -> [-zLen/2(far),zLen/2(near)]
		0, 0, 0, 1;
	t2 <<						//对
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, f2,			// [-zLen/2,zLen/2] -> [min(abs(zF,zN)) (far) ,max(abs(zF,zN)) (near)] (正值,越小越深)
		0, 0, 0, 1;
	return t2 * s * t;
}

inline Vec3 getOrthoVec(const Vec3& v) {
	Vec3 xz(v[0], 0, v[2]);  //v去除y轴在xOz平面上的投影
	Vec3 axis(-xz[2], 0, xz[0]); //与投影垂直的向量
	return toVec3(getRotation(axis, 90) * toVec4(v, 0));
}

inline size_t viewportCoord_to_bufferOffset(int width, int height, int vx, int vy) {
	//视口坐标 to buffer偏移,buffer按照图像坐标系,先计算canvas坐标(y从上到下递减),故需翻转y
	//假设视口高度20像素,则坐标范围[0,19],翻转后19->0 0->19,即canvas_y = 20-1-vp_y
	int canvas_y = (height - 1 - vy);
	return size_t(width) * size_t(canvas_y) + size_t(vx);
}

Vec3 localToWorld(const Vec3& a, const Vec3& N) {
	Vec3 B, C;
	if (fabs(N.x()) > fabs(N.y())) {
		float invLen = 1.0 / sqrt(N.x() * N.x() + N.z() * N.z());
		C = Vec3(N.z() * invLen, 0.0f, -N.x() * invLen);
	} else {
		float invLen = 1.0 / std::sqrt(N.y() * N.y() + N.z() * N.z());
		C = Vec3(0.0, N.z() * invLen, -N.y() * invLen);
	}
	B = C.cross(N);
	return a.x() * B + a.y() * C + a.z() * N;
}

template<typename T>
inline T lerp(const T& b, const T& e, const float& t)
{
	return b * (1 - t) + e * t;
}


template<typename T,typename = typename enable_if<is_same<T,Vec3>::value || is_same<T, Point3>::value>::type>
inline T getMin(const T& a, const  T& b) {
	double x = std::min(a.x(), b.x());
	double y = std::min(a.y(), b.y());
	double z = std::min(a.z(), b.z());
	return T(x, y, z);
}

template<typename T, typename = typename enable_if<is_same<T, Vec3>::value || is_same<T, Point3>::value>::type>
inline T getMin(const T& a, const  T& b,const T& c) {
	double x = std::min({ a.x(), b.x() ,c.x()});
	double y = std::min({ a.y(), b.y(),c.y() });
	double z = std::min({ a.z(), b.z(),c.z() });
	return T(x, y, z);
}


template<typename T, typename = typename enable_if<is_same<T, Vec3>::value || is_same<T, Point3>::value>::type>
inline T getMax(const T& a, const  T& b) {
	double x = std::max(a.x(), b.x());
	double y = std::max(a.y(), b.y());
	double z = std::max(a.z(), b.z());
	return T(x, y, z);
}

template<typename T, typename = typename enable_if<is_same<T, Vec3>::value || is_same<T, Point3>::value>::type>
inline T getMax(const T& a, const  T& b, const T& c) {
	double x = std::max({ a.x(), b.x(),c.x() });
	double y = std::max({ a.y(), b.y() ,c.y() });
	double z = std::max({ a.z(), b.z() ,c.z() });
	return T(x, y, z);
}

#endif // COMMON_UTILITY_H