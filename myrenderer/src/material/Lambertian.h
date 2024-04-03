#pragma once
#ifndef MATERIAL_LAMBERTIAN_H
#define MATERIAL_LAMBERTIAN_H
#include <material/Material.h>
#include <device/Renderer.h>

class Lambertian : public Material {
public:
    shared_ptr<Texture> diffuse;

    Lambertian(const shared_ptr<Texture>& diffuseTexture) :diffuse(diffuseTexture) {}

};

#endif 
