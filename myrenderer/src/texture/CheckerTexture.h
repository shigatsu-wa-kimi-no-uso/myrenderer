#pragma once
#ifndef TEXTURE_CHECKER_TEXTURE_H
#define TEXTURE_CHECKER_TEXTURE_H
#include <texture/Texture.h>

class CheckerTexture : public Texture {
private:
	ColorN _color[2];
public:
	CheckerTexture(){}
	CheckerTexture(const Color255& color1, const Color255& color2){
		setColor(color1, color2);
	}

	//需配合 x + y + z到u v的转换,如果定v恒为0  u =  x + y + z, 即可实现checker图案
	ColorN value(double u, double v) const override {
		double epsilon = 0.001;  //防止误差
		// 如果调用value函数时，在传参时计算，例如value(x+y+z,0) 此时xyz均会被转换为double,此时会造成误差
		//例如3+2+1->3.01+2.99+1.0=6 而4+2+0->4+2.99+0.0=5.99  小数截断后为5 原本应为偶数此时为奇数
		unsigned int trunc = u + v + epsilon;  // 特别注意,如果用int,则对负数取模取%2有可能为-1造成数组越界
		return _color[trunc%2];
	}

	void setColor(const Color255& color1,const Color255& color2) {
		_color[0] = normalizeColor(color1);
		_color[1] = normalizeColor(color2);
	}

};


#endif
