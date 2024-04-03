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
    /* Public Camera Parameters Here */
    int    max_depth = 10;   // Maximum number of ray bounces into scene
    double    frustum_near = 10; //视锥体相关参数（如果使用光栅化，需要设置这个参数，如果用光线追踪则设置focus_dist）
    double    frustum_far = 50;

    double fov = 50;  // Vertical view angle (field of view) (degree)
    Point3 lookfrom = Point3(0, 0, -1);  // Point camera is looking from
    Point3 lookat = Point3(0, 0, 0);   // Point camera is looking at
    Vec3   vup = Vec3(0, 1, 0);     // Camera-relative "up" direction

    double defocus_angle = 0;  // Variation angle of rays through each pixel
    double focus_dist = 10;    // Distance from camera lookfrom point to plane of perfect focus
    double aspect_ratio;

private:
    /* Private Camera Variables Here */
    Point3 _center;         // Camera center
    Point3 _pixel00_loc;    // Location of pixel 0, 0
    Vec3   _pixel_delta_u;  // Offset to pixel to the right
    Vec3   _pixel_delta_v;  // Offset to pixel below
    Vec3   _u, _v, _w;        // Camera frame basis vectors
    Vec3   _defocus_disk_u;  // Defocus disk horizontal radius
    Vec3   _defocus_disk_v;  // Defocus disk vertical radius

};
#endif // DEVICE_CAMERA_H
