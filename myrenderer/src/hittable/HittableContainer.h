#pragma once
#ifndef HITTABLE_HITTABLE_CONTAINER_H
#define HITTABLE_HITTABLE_CONTAINER_H
#include <common/global.h>
#include <hittable/Hittable.h>

class HittableContainer : public Hittable
{
public:
	HittableContainer(){}
    virtual ~HittableContainer() = default;
    virtual void clear() = 0;
    virtual void build(){}
    virtual void add(shared_ptr<Hittable> object) = 0;

};

#endif // !HITTABLE_HITTABLE_CONTAINER_H
