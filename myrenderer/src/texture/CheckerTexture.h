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

	//����� x + y + z��u v��ת��,�����v��Ϊ0  u =  x + y + z, ����ʵ��checkerͼ��
	ColorN value(double u, double v) const override {
		double epsilon = 0.001;  //��ֹ���
		// �������value����ʱ���ڴ���ʱ���㣬����value(x+y+z,0) ��ʱxyz���ᱻת��Ϊdouble,��ʱ��������
		//����3+2+1->3.01+2.99+1.0=6 ��4+2+0->4+2.99+0.0=5.99  С���ضϺ�Ϊ5 ԭ��ӦΪż����ʱΪ����
		unsigned int trunc = u + v + epsilon;  // �ر�ע��,�����int,��Ը���ȡģȡ%2�п���Ϊ-1�������Խ��
		return _color[trunc%2];
	}

	void setColor(const Color255& color1,const Color255& color2) {
		_color[0] = normalizeColor(color1);
		_color[1] = normalizeColor(color2);
	}

};


#endif
