#pragma once
#ifndef COMMON_LIGHT_H
#define COMMON_LIGHT_H
#include <common/global.h>
#include <common/Buffer.h>

class Light {
public:
	Point3 position;
	Vec3 intensity;
    Point3 direction;
    Vec3 up; //模型摄像头参数用 同camera.vup

    //平行光参数
    double width;  
    double aspectRatio;

    double maxDistance;  //光最远到达的半径长度

    

    //shadowmap参数
    ShadowMap shadowMap;

};

#endif 
