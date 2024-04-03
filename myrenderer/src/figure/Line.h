#pragma once
#ifndef FIGURE_LINE_H
#define FIGURE_LINE_H
#include <figure/Figure.h>
#include <functional>

class Line : public Figure {
private:
	Point2 _points[2];
	ColorN _color;
	//digital differential analyzer(��ֵ΢��)����
	std::function<void(const Line&, Canvas&)> _method;

public:
	void setPoints(const Point2& p0, const Point2& p1) {
		_points[0] = p0;
		_points[1] = p1;
		//��֤ point 0�����
		if (_points[0].x() > _points[1].x()) {
			std::swap(_points[0], _points[1]);
		}
	}

	void setColor(const ColorN& color) {
		_color = color;
	}


	void setDrawMethod(std::function<void(const Line&, Canvas&)> method) {
		_method = method;
	}

	void drawOn(Canvas& canvas) const override {
		_method(*this, canvas);
	}


	static void midpoint_method(const Line& line, Canvas& canvas) {
		//��������(0,0)��Ӧʵ������Ϊ(0,0)����(0.5,0.5)
		int x0 = line._points[0].x();
		int x1 = line._points[1].x();
		int y0 = line._points[0].y();
		int y1 = line._points[1].y();
		double delta_y = y1 - y0;
		double delta_x = x1 - x0;
		double k;
		int iter_begin;
		int iter_end;
		double real_dependent_coord;
		int pixel_dependent_coord;
		bool steep;

		if (std::abs(delta_y) > std::abs(delta_x)) {
			//�Ƚ϶�ʱ��y���� y = (dy/dx)*x + b -->  x = (dx/dy)*(y - b) --> x[i+1] = y[i] + dx/dy
			iter_begin = y0;
			iter_end = y1;
			pixel_dependent_coord = x0;
			real_dependent_coord = x0;
			steep = true;
			k = delta_x / delta_y;
		} else {
			//��x���� y = (dy/dx)*x + b  --> y[i+1] = y[i] + dy/dx
			iter_begin = x0;
			iter_end = x1;
			pixel_dependent_coord = y0;
			real_dependent_coord = y0;
			steep = false;
			k = delta_y / delta_x;
		}

		for (int i = iter_begin; i <= iter_end; i++) {
			size_t offset;
			if (steep) {
				//�߶���ʱ,��������y��
				offset = canvas.viewportCoord_to_bufferOffset(pixel_dependent_coord, i);
			} else {
				//�������x��
				offset = canvas.viewportCoord_to_bufferOffset(i, pixel_dependent_coord);
			}
			canvas.drawOnePixel(line._color, offset);
			real_dependent_coord += k;
			//�б�ʽ: ʵ��ֵ���¸���������ָ�������λ�ù�ϵ�ж�
			//�������궨λ���ص�����,������һ���߳�1�������ο�
			//��������һ��+1���i+1,�������������,ֻ����+1���߲���
			//��ʱ����ֱ����i+1��������ʵ��������ֵ,��"��������һ�����صĺ���"���������+1
			//pixel_dependent_coord + 0.5 : ���ؼ���Ϊ1,��+0.5��ζ�ŵ������ؼ�߽�
			pixel_dependent_coord += (real_dependent_coord > pixel_dependent_coord + 0.5) ? 1 : 0;
			// x2�Ż�,ȥ������ӷ�
			//pixel_dependent_coord += (2*real_dependent_coord > 2*pixel_dependent_coord + 1) ? 1 : 0;
		}

	}

	static void bresenham_method(const Line& line, Canvas& canvas) {
		//��������(0,0)��Ӧʵ������Ϊ(0,0)����(0.5,0.5)
		int x0 = line._points[0].x();
		int x1 = line._points[1].x();
		int y0 = line._points[0].y();
		int y1 = line._points[1].y();
		double delta_y = y1 - y0;
		double delta_x = x1 - x0;
		double k;
		int iter_begin;
		int iter_end;
		double error;
		int pixel_dependent_coord;
		bool steep;

		if (std::abs(delta_y) > std::abs(delta_x)) {
			//�Ƚ϶�ʱ��y���� y = (dy/dx)*x + b -->  x = (dx/dy)*(y - b) --> x[i+1] = y[i] + dx/dy
			iter_begin = y0;
			iter_end = y1;
			pixel_dependent_coord = x0;
			steep = true;
			k = delta_x / delta_y;
		} else {
			//��x���� y = (dy/dx)*x + b  --> y[i+1] = y[i] + dy/dx
			iter_begin = x0;
			iter_end = x1;
			pixel_dependent_coord = y0;
			steep = false;
			k = delta_y / delta_x;
		}

		// �Ż�,ȥ������Ƚ�
		//error = -0.5;
		error = 0;

		for (int i = iter_begin; i <= iter_end; i++) {
			size_t offset;
			if (steep) {
				//�߶���ʱ,��������y��
				offset = canvas.viewportCoord_to_bufferOffset(pixel_dependent_coord, i);
			} else {
				//�������x��
				offset = canvas.viewportCoord_to_bufferOffset(i, pixel_dependent_coord);
			}
			canvas.drawOnePixel(line._color, offset);
			error += k;
			//������0.5ʱ��ζ�������õ�ǰpixel_dependent_coord�����ɴ���0.5�����,Ӧ��+1ʹ�����С��0.5
			//�������ʹ���ܹ����ں��ʵ����ط�����
			if (error > 0.5) {
				pixel_dependent_coord++;
				error--;
			} 
			// �Ż�,ȥ������Ƚ� (���error = -0.5��ʼֵ)
			/*
			if (error > 0) {
				pixel_dependent_coord++;
				error--;
			}*/
			//pixel_dependent_coord += (2*real_dependent_coord > 2*pixel_dependent_coord + 1) ? 1 : 0;
		}
	}


	static void dda_method(const Line& line,Canvas& canvas) {
		//��������(0,0)��Ӧʵ������Ϊ(0,0)����(0.5,0.5)
		int x0 = line._points[0].x();
		int x1 = line._points[1].x();
		int y0 = line._points[0].y();
		int y1 = line._points[1].y();
		double delta_y = y1 - y0;
		double delta_x = x1 - x0;
		double k;
		int iter_begin;
		int iter_end;
		double dependent_coord;
		bool steep;

		if (std::abs(delta_y) > std::abs(delta_x)) {
			//�Ƚ϶�ʱ��y���� y = (dy/dx)*x + b -->  x = (dx/dy)*(y - b) --> x[i+1] = y[i] + dx/dy
			iter_begin = y0;
			iter_end = y1;
			dependent_coord = x0;
			steep = true;
			k = delta_x / delta_y;
		} else {
			//��x���� y = (dy/dx)*x + b  --> y[i+1] = y[i] + dy/dx
			iter_begin = x0;
			iter_end = x1;
			dependent_coord = y0;
			steep = false;
			k = delta_y / delta_x;
		}

		for (int i = iter_begin; i <= iter_end; i++) {
			size_t offset;
			if (steep) {
				//�߶���ʱ,��������y��
				offset = canvas.viewportCoord_to_bufferOffset(dependent_coord, i);
			} else {
				//�������x��
				offset = canvas.viewportCoord_to_bufferOffset( i, dependent_coord);
			}
			canvas.drawOnePixel(line._color, offset);
			dependent_coord += k;
		}
	}
};

#endif // LINE_H
