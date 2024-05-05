#pragma once
#ifndef DEVICE_CAMERA_H
#define DEVICE_CAMERA_H
#include <iostream>
#include <hittable/Hittable.h>
#include <common/global.h>
#include <common/Interval.h>
#include <common/Ray.h>



class Camera
{
public:
    int maxDepth = 10;   // Maximum number of ray bounces into scene
    double frustumNear = 10; //视锥体相关参数（如果使用光栅化，需要设置这个参数，如果用光线追踪则设置focus_dist）
    double frustumFar = 50;

    double fov = 50;  // Vertical view angle (field of view) (degree)
    Point3 lookfrom = Point3(0, 0, -1);  // Point camera is looking from
    Point3 lookat = Point3(0, 0, 0);   // Point camera is looking at
    Vec3   vup = Vec3(0, 1, 0);     // Camera-relative "up" direction 相当于摄像头t向量,但不一定与g垂直,后续会修正 这个量只起方向指导作用

    double defocusAngle = 0;  // Variation angle of rays through each pixel
    double focusDist = 10;    // Distance from camera lookfrom point to plane of perfect focus
    double aspectRatio;

private:
    Point3 _center;         // Camera center
    Vec3   _gxt, _t, _z;        // Camera frame basis vectors  x轴,y轴,z轴
    Vec3   _defocusDiscU;  // Defocus disc horizontal radius
    Vec3   _defocusDiscV;  // Defocus disc vertical radius
    double _defocusAngle;


public:
    void initialize() {
        _center = lookfrom; //摄像头中心
        Vec3 g = (lookat - lookfrom).normalized(); //g轴
        _z = -g;  //z轴
        _gxt = vup.cross(_z).normalized(); //gxt
        _t = _z.cross(_gxt);    //真正的t向量

        // Calculate the camera defocus disc basis vectors.
        double defocusRadius = focusDist * tan(deg2rad(defocusAngle / 2));
        _defocusDiscU = _gxt * defocusRadius;  //与视野的u v轴不是一回事,仅仅是为了方便随机采样
        _defocusDiscV = _t * defocusRadius;
    }

    Point3 sampleInDefocusDisc() const {
        // Returns a random point in the camera defocus disc.
        if (_defocusAngle <= 0) {
            return _center;
        }
        Vec2 p = random_in_unit_circle(); //在单位圆中随机取点
        return _center + (p[0] * _defocusDiscU) + (p[1] * _defocusDiscV);
    }
};
#endif // DEVICE_CAMERA_H
