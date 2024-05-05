#pragma once
#ifndef HITTABLE_TRIANGLE_H
#define HITTABLE_TRIANGLE_H
#include <hittable/Hittable.h>


class Triangle : public Hittable {
private:
    bool _hasIntersection(const Ray& ray,double& t) const{
        // Moller - Trumbore
        Vec3 s = ray.origin - vertices_vec3[0];
        Vec3 s1 = ray.direction.cross(e2);
        Vec3 s2 = s.cross(e1);
        Vec3 sol = 1 / s1.dot(e1) * Vec3(s2.dot(e2), s1.dot(s), s2.dot(ray.direction));
        double tnear = sol.x();
        double u = sol.y();
        double v = sol.z();
        if (tnear >= 0 && u >= 0 && v >= 0 && 1 - u - v >= 0) {
            t = tnear;
            return true;
        }
        return false;
    }

    void _calculateTangent() {
        for (int i = 0; i < 3; i++) {
            Vec2 delta2 = texCoords[(i + 2) % 3] - texCoords[i];
            Vec2 delta1 = texCoords[(i + 1) % 3] - texCoords[i];
            double ratio = 1.0 / (delta1[0] * delta2[1] - delta2[0] * delta1[1]);
            Vec3 tangent = ratio * (delta2[1] * e1 - delta1[1] * e2);
            Vec3 n = toVec3(normals[i]).normalized();
            tangents[i] = (tangent - tangent.dot(n) * n).normalized(); //正交化
        }
    }

public:
	bool hit(const Ray& r, const Interval& range, HitRecord& rec) const override{
        double t;
        if (_hasIntersection(r, t)) {
            if (t > range.max) {
                return false;
            }
            rec.hitAnything = true;
            rec.point = r.at(t);
            rec.normal = normal;
            rec.t = t;
            rec.material = material;
            rec.frontFace = normal.dot(r.direction) > 0 ? false : true; 
            return true;
        }
		return false;
	}

   void samplePoint(HitRecord& rec, double& pdf) const override{
        double x = sqrt(random_double()), y = random_double();
        rec.point = vertices_vec3[0] * (1.0 - x) + vertices_vec3[1] * (x * (1.0 - y)) + vertices_vec3[2] * (x * y);
        rec.normal = normal;
        pdf = 1.0f / area;
    }


    void calculateOthers() override {
        vertices_vec3[0] = toVec3(vertices[0]);
        vertices_vec3[1] = toVec3(vertices[1]);
        vertices_vec3[2] = toVec3(vertices[2]);
        e1 = vertices_vec3[1] - vertices_vec3[0];
        e2 = vertices_vec3[2] - vertices_vec3[0];
        normal = e1.cross(e2).normalized();
        area = e1.cross(e2).norm() * 0.5;
        _calculateTangent(); //本应从文件中读取,由于文件中没有,所以在这里计算
    }


    double getArea() const override {
        return area;
    }

    Bounds3 getBoundingBox() const override {
        Point3 minPnt = getMin(vertices_vec3[0], vertices_vec3[1],vertices_vec3[2]);
        Point3 maxPnt = getMax(vertices_vec3[0], vertices_vec3[1], vertices_vec3[2]);
        return Bounds3(minPnt, maxPnt);
    }

    static void calculateTangent(Triangle& t) {
        for (int i = 0; i < 3; i++) {
            Vec3 e1 = toVec3(t.vertices[(i + 1) % 3] - t.vertices[i]);
            Vec3 e2 = toVec3(t.vertices[(i + 2) % 3] - t.vertices[i]);
            Vec2 delta2 = t.texCoords[(i + 2) % 3] - t.texCoords[i];
            Vec2 delta1 = t.texCoords[(i + 1) % 3] - t.texCoords[i];
            double ratio = 1.0 / (delta1[0] * delta2[1] - delta2[0] * delta1[1]);
            Vec3 tangent = ratio * (delta2[1] * e1 - delta1[1] * e2);
            Vec3 n = toVec3(t.normals[i]).normalized();
            t.tangents[i] = (tangent - tangent.dot(n) * n).normalized(); //正交化
        }
    }

    double area;
    Vec3 e1;
    Vec3 e2;
	Vec4 vertices[3];
	Point3 vertices_vec3[3];
	ColorN colors[3];
	Point2 texCoords[3]; //贴图uv
	Vec4 normals[3]; //顶点法向量
    Vec3 normal; //面法向量
	shared_ptr<Material> material;
	Vec3 tangents[3];	//切向量(构成TBN空间)
};


#endif
