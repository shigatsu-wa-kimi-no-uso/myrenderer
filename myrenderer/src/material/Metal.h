#pragma once
#ifndef MATERIAL_METAL_H
#define MATERIAL_METAL_H
#include <material/Material.h>


class Metal : public Material{
public:
    ColorN specular; //Ks
    double p;          //pÖ¸Êý (Is*pow(cos<n,h>,p)
    Metal(const ColorN& specular, double p) : specular(specular), p(p) {

    }
};

#endif // MATERIAL_METAL_H
