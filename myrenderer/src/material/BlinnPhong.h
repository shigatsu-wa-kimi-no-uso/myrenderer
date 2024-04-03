#pragma once
#ifndef MATERIAL_BLINN_PHONG_H
#define  MATERIAL_BLINN_PHONG_H
#include <texture/SolidColorTexture.h>

class BlinnPhong : public Material {
public:
	//Ka
	ColorN ambient;
	ColorN specular; //Ks
	double p;          //pÖ¸Êý (Is*pow(cos<n,h>,p)
	shared_ptr<Texture> diffuse; //Kd
	BlinnPhong(const ColorN& ambient, const ColorN& specular, double p, const shared_ptr<Texture>& diffuseTexture)
		:diffuse(diffuseTexture), specular(specular), p(p), ambient(ambient) {}

};

#endif
