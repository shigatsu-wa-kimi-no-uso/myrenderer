#pragma once
#ifndef RENDERER_H
#define RENDERER_H
#include <common/global.h>
#include <device/shader/Shader.h>
#include <device/Camera.h>
#include <hittable/MeshModel.h>
#include <device/Rasterizer.h>
#include <iomanip>
class Renderer {
private:
	struct Renderable {
		shared_ptr<MeshModel> meshMod;
		shared_ptr<Mat4> modeling;
	};
	using ItemMap = std::unordered_map<int, Renderable>;
	shared_ptr<Camera> _camera; 
	shared_ptr<Shader> _shader;
	shared_ptr<Canvas> _canvas;
	shared_ptr<Rasterizer> _rasterizer;
	ItemMap _itemMap;
	int _lastIndex;
	Mat4 _view;
	Mat4 _projection;
	Mat4 _viewport;
	Mat4 _defaultModeling = Mat4::Identity();
	std::function<void(Renderer*)> _renderMethod;
	double _aspectRatio;
	int    _width;  // 对输出图像,即画布而言,并非视口的宽度！
	int    _samplesPerPixel;   // 输出图像每像素采样数


	void _updateViewMatrix() {
		Vec3 eye_pos = _camera->lookfrom;
		Mat4 translate;
		translate << 1, 0, 0, -eye_pos[0],
			0, 1, 0, -eye_pos[1],
			0, 0, 1, -eye_pos[2],
			0, 0, 0, 1;
		Vec3 g = _camera->lookat.normalized();
		Vec3 t = _camera->vup.normalized();
		Vec3 gxt = g.cross(t);
		Mat4 view;
		view << gxt[0], gxt[1], gxt[2], 0,
			t[0], t[1], t[2], 0,
			-g[0], -g[1], -g[2], 0,
			0, 0, 0, 1;
		view = translate * view;
		_view = view;
	}	
	
	void _updateProjectionMatrix() {
		// 远近与[-1,1]的关系: zFar --> -1 zNear --> 1 
		Mat4 projection;
		double f = -_camera->frustum_far, n = -_camera->frustum_near;
		double t = std::tan(pi * (_camera->fov / 2.0) / 180.0) * std::abs(n);
		double b = -t, r = t * _camera->aspect_ratio, l = -r;  //基于世界坐标系的b,t,l,r  与canvas的大小即输出图像的大小不是一回事
		// camera的aspect_ratio与输出图像的aspect ratio可能不相同
		Mat4 ortho, trans, scale, persp2ortho;
		scale <<
			2 / (r - l), 0, 0, 0,
			0, 2 / (t - b), 0, 0,
			0, 0, 2 / (n - f), 0,
			0, 0, 0, 1;
		trans <<
			1, 0, 0, -(r + l) / 2,
			0, 1, 0, -(t + b) / 2,
			0, 0, 1, -(n + f) / 2,
			0, 0, 0, 1;
		persp2ortho <<
			n, 0, 0, 0,
			0, n, 0, 0,
			0, 0, n + f, -(n * f),
			0, 0, 1, 0;
		// 视锥体->标准正方体[-1,1]^3
		ortho = scale * trans;
		//标准正方体->透视转正交
		projection = ortho * persp2ortho;
		_projection = projection;
	}

	void _updateViewportMatrix(){
		//注意: 如果对z轴操作,则可能改变z轴值的大小符号与远近的关系
		//此变换中, ndc_z = -1 --> screen_z=zNear, ndc_z = 1 --> screen_z=zFar, 0 < zNear < zFar
		double zFar = -_camera->frustum_far, zNear = -_camera->frustum_near;
		double f1 = (zFar - zNear) / 2.0;
		double f2 = (zFar + zNear) / 2.0;
		double width = _width;
		double height = _width /_aspectRatio;
		Mat4 t, s, t2;
		//[-1,1] -->[0,2]
		t <<
			1, 0, 0, 1,  
			0, 1, 0, 1,
			0, 0, 1, 0,
			0, 0, 0, 1;
		s <<
			0.5 * width, 0, 0, 0,	//[0,2]^2->[0,width]*[0,height]
			0, 0.5 * height, 0, 0,
			0, 0, f1, 0,			//[-zLen/2,zLen/2]
			0, 0, 0, 1;
		t2 <<						//对
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, f2,			//[zF,zN]
			0, 0, 0, 1;
		_viewport = t2 * s * t;
	}

	void _configDevices() {
		int height = _width / _aspectRatio;
		_rasterizer = make_shared<Rasterizer>();
		_canvas = make_shared<Canvas>();
		_rasterizer->setFrameSize(_width,height);
		_canvas->setCanvasSize(_width, height);
	}

public:
	enum RenderMethod {
		RASTERIZATION = 0,
		RAY_TRACING = 1
	};

	void setCanvasAspectRatio(double aspectRatio) {
		_aspectRatio = aspectRatio;
	}

	void setCanvasWidth(int width) {
		_width = width;
	}

	void setSamplesPerPixel(int samplesPerPixel) {
		_samplesPerPixel = samplesPerPixel;
	}

	void setCamera(const shared_ptr<Camera>& camera) {
		_camera = camera;
		_updateViewMatrix();
		_updateProjectionMatrix();
		_updateViewportMatrix();
		_configDevices();
	}

	void setShader(const shared_ptr<Shader>& shader) {
		_shader = shader;
	}

	void setSSAAMultiple(int multiple) {
		_rasterizer->setSSAAMultiple(multiple);
	}

	int addMeshModel(const shared_ptr<MeshModel>& meshMod) {
		_itemMap[++_lastIndex] = { meshMod,make_shared<Mat4>(_defaultModeling)};
		return _lastIndex;
	}

	void setModelTranslation(int meshModIdx,const shared_ptr<Mat4>& mt) {
		_itemMap[meshModIdx].modeling = mt;
	}


	void removeMeshModel(int meshModIdx) {
		_itemMap.erase(meshModIdx);
	}
	
	size_t modelCount() {
		return _itemMap.size();
	}

	void setRenderMethod(RenderMethod method) {
		switch (method) {
		case RASTERIZATION:
			_renderMethod = &Renderer::rasterize_method;
			break;
		case RAY_TRACING:
			break;
		}

	}

	void rasterize_method() {
		int modelCnt = modelCount();
		for (const pair<const int, Renderable> entry : _itemMap) {
			const Renderable& item = entry.second;
			_shader->uni.model = *item.modeling;
			_shader->uni.view = _view;
			_shader->uni.projection = _projection;
			int triCnt = item.meshMod->meshList.size();
			modelCnt--;
			for (const Triangle& t : item.meshMod->meshList) {
				triCnt--;
				if (triCnt == 5800) {
					int a = 5;
				}
				std::clog << "\rModels remaining: " << std::setw(5)  << modelCnt<<" Current model meshes remaining: " <<std::setw(5) << triCnt << std::flush;
				Vec4 screenCoords[3];
				for (int i = 0; i < 3; i++) {
					//for each vertex: Vertex Shader -> Homogeneous divide -> Viewport transform
					//space transform: object space --MVP--> clipping space --divide by w--> NDC --viewport trans--> screen space
					Vec4 clipCoord;
					Vec3 vertexCoord = toVec3(t.vertices[i]);
					Vec3 normal = toVec3(t.normals[i]);
					_shader->attr.vertexCoord = vertexCoord;
					_shader->attr.normal = normal;
					_shader->shadeVertex(i, clipCoord);
					//裁剪操作暂时忽略
					Vec4 ndcCoord = clipCoord / clipCoord.w();
					screenCoords[i] = _viewport * ndcCoord;
				}
				_rasterizer->rasterize_triangle(screenCoords,_shader);
			
			}
		}
		std::clog << "\n";
		std::clog << "Drawing..." << std::flush;
		_rasterizer->drawOnCanvas(_canvas);
	}

	void render() {
		_renderMethod(this);
	}

	void clearBuffer() {
		_rasterizer->clearBuffer();
		_canvas->clearBuffer();
	}

	void output(std::ostream& out) {
		_canvas->outputPixelBuffer(out);
	}

};

#endif // RENDERER_H
