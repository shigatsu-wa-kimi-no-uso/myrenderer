#pragma once
#ifndef COMMON_HIT_RECORD_H
#define COMMON_HIT_RECORD_H
#include <common/global.h>

class Material;

struct HitRecord
{
    Point3 point;
    Vec3 normal;
    shared_ptr<Material> material;
    double t;
    bool front_face;
};

#endif
