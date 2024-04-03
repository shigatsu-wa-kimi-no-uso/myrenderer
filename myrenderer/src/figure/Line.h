#pragma once
#ifndef FIGURE_LINE_H
#define FIGURE_LINE_H
#include <figure/Figure.h>
#include <functional>

class Line : public Figure {
private:
	Point2 _points[2];
	ColorN _color;
	//digital differential analyzer(数值微分)方法
	std::function<void(const Line&, Canvas&)> _method;

public:
	void setPoints(const Point2& p0, const Point2& p1) {
		_points[0] = p0;
		_points[1] = p1;
		//保证 point 0在左侧
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
		//假设像素(0,0)对应实际坐标为(0,0)而非(0.5,0.5)
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
			//比较陡时对y迭代 y = (dy/dx)*x + b -->  x = (dx/dy)*(y - b) --> x[i+1] = y[i] + dx/dy
			iter_begin = y0;
			iter_end = y1;
			pixel_dependent_coord = x0;
			real_dependent_coord = x0;
			steep = true;
			k = delta_x / delta_y;
		} else {
			//对x迭代 y = (dy/dx)*x + b  --> y[i+1] = y[i] + dy/dx
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
				//线陡峭时,迭代的是y轴
				offset = canvas.viewportCoord_to_bufferOffset(pixel_dependent_coord, i);
			} else {
				//否则迭代x轴
				offset = canvas.viewportCoord_to_bufferOffset(i, pixel_dependent_coord);
			}
			canvas.drawOnePixel(line._color, offset);
			real_dependent_coord += k;
			//判别式: 实际值与下个迭代坐标指向的像素位置关系判断
			//像素坐标定位像素的中心,像素是一个边长1的正方形框
			//迭代坐标一定+1变成i+1,而对于因变坐标,只可能+1或者不变
			//此时计算直线在i+1迭代的真实因变坐标的值,若"进入了下一个像素的盒子"则因变坐标+1
			//pixel_dependent_coord + 0.5 : 像素间间距为1,故+0.5意味着到达像素间边界
			pixel_dependent_coord += (real_dependent_coord > pixel_dependent_coord + 0.5) ? 1 : 0;
			// x2优化,去除浮点加法
			//pixel_dependent_coord += (2*real_dependent_coord > 2*pixel_dependent_coord + 1) ? 1 : 0;
		}

	}

	static void bresenham_method(const Line& line, Canvas& canvas) {
		//假设像素(0,0)对应实际坐标为(0,0)而非(0.5,0.5)
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
			//比较陡时对y迭代 y = (dy/dx)*x + b -->  x = (dx/dy)*(y - b) --> x[i+1] = y[i] + dx/dy
			iter_begin = y0;
			iter_end = y1;
			pixel_dependent_coord = x0;
			steep = true;
			k = delta_x / delta_y;
		} else {
			//对x迭代 y = (dy/dx)*x + b  --> y[i+1] = y[i] + dy/dx
			iter_begin = x0;
			iter_end = x1;
			pixel_dependent_coord = y0;
			steep = false;
			k = delta_y / delta_x;
		}

		// 优化,去除浮点比较
		//error = -0.5;
		error = 0;

		for (int i = iter_begin; i <= iter_end; i++) {
			size_t offset;
			if (steep) {
				//线陡峭时,迭代的是y轴
				offset = canvas.viewportCoord_to_bufferOffset(pixel_dependent_coord, i);
			} else {
				//否则迭代x轴
				offset = canvas.viewportCoord_to_bufferOffset(i, pixel_dependent_coord);
			}
			canvas.drawOnePixel(line._color, offset);
			error += k;
			//误差大于0.5时意味着若沿用当前pixel_dependent_coord则会造成大于0.5的误差,应该+1使得误差小于0.5
			//控制误差使得能够画在合适的像素方格中
			if (error > 0.5) {
				pixel_dependent_coord++;
				error--;
			} 
			// 优化,去除浮点比较 (配合error = -0.5初始值)
			/*
			if (error > 0) {
				pixel_dependent_coord++;
				error--;
			}*/
			//pixel_dependent_coord += (2*real_dependent_coord > 2*pixel_dependent_coord + 1) ? 1 : 0;
		}
	}


	static void dda_method(const Line& line,Canvas& canvas) {
		//假设像素(0,0)对应实际坐标为(0,0)而非(0.5,0.5)
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
			//比较陡时对y迭代 y = (dy/dx)*x + b -->  x = (dx/dy)*(y - b) --> x[i+1] = y[i] + dx/dy
			iter_begin = y0;
			iter_end = y1;
			dependent_coord = x0;
			steep = true;
			k = delta_x / delta_y;
		} else {
			//对x迭代 y = (dy/dx)*x + b  --> y[i+1] = y[i] + dy/dx
			iter_begin = x0;
			iter_end = x1;
			dependent_coord = y0;
			steep = false;
			k = delta_y / delta_x;
		}

		for (int i = iter_begin; i <= iter_end; i++) {
			size_t offset;
			if (steep) {
				//线陡峭时,迭代的是y轴
				offset = canvas.viewportCoord_to_bufferOffset(dependent_coord, i);
			} else {
				//否则迭代x轴
				offset = canvas.viewportCoord_to_bufferOffset( i, dependent_coord);
			}
			canvas.drawOnePixel(line._color, offset);
			dependent_coord += k;
		}
	}
};

#endif // LINE_H
