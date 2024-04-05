#pragma once
#ifndef MATERIAL_BLINN_PHONG_WITH_BUMP_H
#define  MATERIAL_BLINN_PHONG_WITH_BUMP__H
#include <material/BlinnPhong.h>
#include <texture/Texture.h>

class BlinnPhongWithBump : public BlinnPhong {
public:
	shared_ptr<Texture> bump;
	BlinnPhongWithBump(const ColorN& ambient, const ColorN& specular, double p, const shared_ptr<Texture>& diffuseTexture,const shared_ptr<Texture>& bumpTexture)
		:BlinnPhong(ambient,specular,p,diffuseTexture), bump(bumpTexture) {}
	BlinnPhongWithBump(const BlinnPhong& blinnPhong, const shared_ptr<Texture>& bumpTexture)
		:BlinnPhong(blinnPhong), bump(bumpTexture) {}

};

#endif
