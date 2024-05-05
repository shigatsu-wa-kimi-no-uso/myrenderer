#pragma once
#ifndef COMMON_BOUNDS3_H
#define COMMON_BOUNDS3_H
#include <common/global.h>
#include <common/utility.h>

class Bounds3
{
public:
	Point3 minPnt;
	Point3 maxPnt;

	Bounds3():maxPnt(-infinity,-infinity,-infinity),minPnt(infinity,infinity,infinity){}
    Bounds3(const Point3& minPnt,const Point3 maxPnt):minPnt(minPnt),maxPnt(maxPnt){}
    Vec3 diagonal() const { return maxPnt - minPnt; }

    //获得值最大的维度序号
    int maxExtent() const
    {
        Vec3 d = diagonal();
        if (d.x() > d.y() && d.x() > d.z())
            return 0;
        else if (d.y() > d.z())
            return 1;
        else
            return 2;
    }
    


    static Bounds3 getUnion(const Bounds3& a, const Bounds3& b) {
        Point3 uMin = getMin(a.minPnt, b.minPnt);
        Point3 uMax = getMax(a.maxPnt, b.maxPnt);
        return Bounds3(uMin, uMax);
    }


    double surfaceArea() const
    {
        Vec3 d = diagonal();
        return 2 * (d.x() * d.y() + d.x() * d.z() + d.y() * d.z());
    }
    
    Vec3 centroid() { return 0.5 * minPnt + 0.5 * maxPnt; }

    void correctNarrowInterval(Point3& pMin,Point3& pMax,double epsilon) {
        //防止BBox薄成一张纸片,影响判断
        Vec3 delta = pMax - pMin;
        for (int i = 0; i < 3; i++) {
            if (delta(i) < epsilon) {
                pMax(i) += epsilon/2;
                pMin(i) -= epsilon/2;
            }
        }
    }


    bool hasIntersection(const Ray& r) {
        Point3 enter = minPnt;
        Point3 exit = maxPnt;
        correctNarrowInterval(enter, exit,0.5);
        //判断光线方向是否为负,若为负,则对应轴的入点和出点大小相反
        for (int i = 0; i < 3; i++) {
            if (r.direction(i) < 0) {
                std::swap(enter(i), exit(i));
            }
        }
        Vec3 invDir = r.direction.cwiseInverse();
        Point3 tEnter = (enter - r.origin).cwiseProduct(invDir);
        Point3 tExit = (exit - r.origin).cwiseProduct(invDir);
        double tEnterMax = std::max({ tEnter(0),tEnter(1),tEnter(2) });
        double tExitMin = std::min({ tExit(0),tExit(1),tExit(2) });
        if (tEnterMax <= tExitMin && tExitMin > 0) {
            return true;
        }
        return false;
    }

	~Bounds3(){}



};



#endif // !COMMON_BOUNDS3_H
