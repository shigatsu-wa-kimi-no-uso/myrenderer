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
    Vec3 up; //ģ������ͷ������ ͬcamera.vup

    //ƽ�й����
    double width;  
    double aspectRatio;

    double maxDistance;  //����Զ����İ뾶����

    

    //shadowmap����
    ShadowMap shadowMap;

};

#endif 
