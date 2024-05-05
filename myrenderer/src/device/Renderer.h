#pragma once
#ifndef DEVICE_RENDERER_H
#define DEVICE_RENDERER_H
#include <common/global.h>
#include <device/shader/Shader.h>
#include <device/Camera.h>
#include <hittable/MeshModel.h>
#include <device/Rasterizer.h>
#include <iomanip>
#include <texture/Texture.h>
#include <device/shader/DepthShader.h>
#include <device/RayTracer.h>
#include <common/HitRecord.h>

class Renderer {
public:
	enum RenderingMethod {
		RASTERIZATION = 1,
		RAY_TRACING = 2,
		MONTE_CARLO_RAY_TRACING = 3,
		RASTERIZATION_WITH_MCRT = 4
	};

	enum RendererConfig {
		WITH_SHADOW = 1,
		USE_BVH = 2
	};

private:
	struct Renderable {
		shared_ptr<HittableContainer> meshMod;	//每个三角形面可能有自己的特殊材质
		shared_ptr<Mat4> modeling;
		shared_ptr<Material> material;	//模型与材质shader之间是多对多关系 模型的全局材质(暂且假定模型的所有面材质相同)
		shared_ptr<Shader> shader;		//模型的全局shader(暂且假定模型的所有面使用同一个shader)
	};
	std::vector<Light> _lights;

	// 光源视角下的 vp*p*view 
	// 在fragment shader中对于每个viewspace中的点均被施加了v*m变换,故对其使用 v^(-1)逆变换,再使用从光源视角渲染时所用的vp*p*view 即可得到这个点在shadowMap中的坐标
	std::vector<Mat4> _objWorld2ShadowMap;
	std::vector<Mat4> _objWorld2LightViewspace;
	
	using ItemMap = std::unordered_map<int, Renderable>;
	shared_ptr<Camera> _camera; 
	shared_ptr<Canvas> _canvas;
	shared_ptr<Scene> _scene;
	shared_ptr<Rasterizer> _rasterizer;
	shared_ptr<RayTracer> _rayTracer;

	ItemMap _itemMap;
	int _lastIndex;
	Mat4 _view;
	Mat4 _projection;
	Mat4 _viewport;
	Mat4 _defaultModeling = Mat4::Identity();
	std::function<void(Renderer*)> _renderMethod;
	std::function<void(Renderer*)> _renderShadowMethod;
	double _aspectRatio;
	int _width;  // 对输出图像,即画布而言,并非视口的宽度！
	int _ssaaMultiple;
	int _samplesPerPixel;   // 输出图像每像素采样数
	bool _withShadow;
	size_t _config = 0;
	RenderingMethod _method;
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
		double f = _camera->frustumFar, n = _camera->frustumNear;			// 定义 camera 在camera坐标系下面向 -z 轴, 越远越小
		double t = std::tan(pi * (_camera->fov / 2.0) / 180.0) * std::abs(n);
		double b = -t, r = t * _camera->aspectRatio, l = -r;  //基于世界坐标系的b,t,l,r  与canvas的大小即输出图像的大小不是一回事
		// camera的aspect_ratio与输出图像的aspect ratio可能不相同
		Mat4 ortho, trans, scale, persp2ortho;
		scale <<							// f -> 2*f/len(n,f)   n -> 2*n/len(n,f)
			2 / (r - l), 0, 0, 0,			 // len(2*f/len(n,f),2*n/len(n,f) ) = 2
			0, 2 / (t - b), 0, 0,
			0, 0, 2 / abs(n - f), 0,	
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
		// 视锥体->裁剪空间				// [n,f] -> [-1*w,1*w]
		ortho = scale * trans;
		//标准正方体->透视转正交
		projection = ortho * persp2ortho;
		_projection = projection;
	}

	void _updateViewportMatrix(){
		//注意: 如果对z轴操作,则可能改变z轴值的大小符号与远近的关系
		//此变换中, ndc_z = -1 --> screen_z=zNear, ndc_z = 1 --> screen_z=zFar,  zFar < zNear < 0
		//输入: NDC([-1,1]^3)   near = -1, far = 1
		double zFar = _camera->frustumFar, zNear = _camera->frustumNear; 
		double f1 = abs(zFar - zNear) / 2.0; // zLen/2.0
		double f2 = abs(zFar + zNear) / 2.0; // 有绝对值！
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
			0, 0, f1, 0,			//[-1(far),1(near)] -> [-zLen/2(far),zLen/2(near)]
			0, 0, 0, 1;
		t2 <<						//对
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, f2,			// [-zLen/2,zLen/2] -> [min(abs(zF,zN)) (far) ,max(abs(zF,zN)) (near)] (正值,越小越深)
			0, 0, 0, 1;				
		_viewport = t2 * s * t;
	}

	void _configDevices() {
		int height = _width / _aspectRatio;
		_rasterizer->useInternalBuffer();
		_rasterizer->setSSAAMultiple(_ssaaMultiple);
		_rasterizer->setFrameSize(_width,height);
		_canvas->setCanvasSize(_width, height);
		clearBuffer();
	}



	void _rasterizeShadowMap() {
		int modelCnt = modelCount();
		Shader::uni.projection = _projection;
		for (const pair<const int, Renderable> entry : _itemMap) {
			const Renderable& item = entry.second;
			Shader::uni.modelView = _view * (*item.modeling);
			const shared_ptr<MeshModel> meshMod = *reinterpret_cast<const shared_ptr<MeshModel>*>(&item.meshMod);
			int triCnt = meshMod->meshList.size();
			int remainCnt = triCnt - 1;
			modelCnt--;
		#pragma omp parallel for
			for (int i = 0; i < triCnt; i++) {
				shared_ptr<Triangle> t = meshMod->meshList[i];
				const shared_ptr<Shader> shader = make_shared<DepthShader>();
			#pragma omp critical
				{
					std::clog << "\rModels remaining: " << std::setw(5) << modelCnt << " Current model meshes remaining: " << std::setw(5) << remainCnt << std::flush;
				}
				Vec4 screenCoords[3];
				for (int i = 0; i < 3; i++) {
					//for each vertex: Vertex Shader -> Homogeneous divide -> Viewport transform
					//space transform: object space --MVP--> clipping space --divide by w--> NDC --viewport trans--> screen space
					Vec4 clipCoord;
					shader->attr.vertexCoord = toVec3(t->vertices[i]);
					shader->shadeVertex(i, clipCoord);
					//裁剪操作暂时忽略
					Vec4 ndcCoord = clipCoord;  // / clipCoord.w(); //使用正交投影不需要除
					screenCoords[i] = _viewport * ndcCoord;
				}
				_rasterizer->rasterize_triangle(screenCoords, shader);
			#pragma omp atomic
				remainCnt--;
			}
		}
		std::clog << "\n";
	}

	void _disposeModels() {
		
	}

	using MaterialMap = std::unordered_map<int, shared_ptr<Material>>;
	using ShaderMap = std::unordered_map<int, shared_ptr<Shader>>;
	using TextureMap = std::unordered_map<int, shared_ptr<Texture>>;

	MaterialMap _materialMap;
	ShaderMap _shaderMap;
	TextureMap _textureMap;



public:
	Renderer() :_rasterizer(make_shared<Rasterizer>()),_canvas(make_shared<Canvas>()),_rayTracer(make_shared<RayTracer>()){}


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
	}

	void setScene(const shared_ptr<Scene>& scene) {
		_scene = scene;
	}

	void rasterizerConfigs() {
		//_updateViewMatrix();
		//_updateProjectionMatrix();
		//_updateViewportMatrix();
		int height = _width / _aspectRatio;
		_view = getViewing(_camera->lookfrom, _camera->lookat, _camera->vup);
		_projection = getProjection(_camera->frustumFar, _camera->frustumNear, _camera->fov, _camera->aspectRatio);
		_viewport = getViewport(_width, _width * _aspectRatio, _camera->frustumFar, _camera->frustumNear);
		_rasterizer->useInternalBuffer();
		_rasterizer->setSSAAMultiple(_ssaaMultiple);
		_rasterizer->setFrameSize(_width, height);
		_canvas->setCanvasSize(_width, height);
		clearBuffer();
	}

	void setSSAAMultiple(int multiple) {
		_ssaaMultiple = multiple;
	}

	int addMeshModel(const shared_ptr<MeshModel>& meshMod) {
		_itemMap[++_lastIndex] = { meshMod,make_shared<Mat4>(_defaultModeling)};
		return _lastIndex;
	}

	int addMeshModel(const shared_ptr<MeshModel_BVH>& meshMod) {
		_itemMap[++_lastIndex] = { meshMod,make_shared<Mat4>(_defaultModeling) };
		return _lastIndex;
	}

	int addMaterial(const shared_ptr<Material>& material) {
		_materialMap[++_lastIndex] = material;
		return _lastIndex;
	}

	int addShader(const shared_ptr<Shader>& shader) {
		_shaderMap[++_lastIndex] = shader;
		return _lastIndex;
	}

	int addTexture(const shared_ptr<Texture>& texture) {
		_textureMap[++_lastIndex] = texture;
		return _lastIndex;
	}

	void addLight(const Light& light) {
		_lights.push_back(light);
	}

	shared_ptr<Texture> getTexture(int textureId) {
		return _textureMap[textureId];
	}

	void bindModelProperties(int renderableId, int shaderId,int materialId) {
		setModelMaterial(renderableId, materialId);
		setModelShader(renderableId, shaderId);
	}

	void setModelTransform(int meshModIdx,const shared_ptr<Mat4>& mt) {
		_itemMap[meshModIdx].modeling = mt;
	}

	void setModelShader(int meshModIdx,int shaderId) {
		_itemMap[meshModIdx].shader = _shaderMap[shaderId];
	}

	void setModelMaterial(int meshModIdx, int materialId) {
		_itemMap[meshModIdx].material = _materialMap[materialId];
	}

	void removeMeshModel(int meshModIdx) {
		_itemMap.erase(meshModIdx);
	}
	
	size_t modelCount() {
		return _itemMap.size();
	}

	void rayTracerConfigs() {
		int height = _width / _aspectRatio;
		_rayTracer->initialize(_camera, _width, height);
		_canvas->setCanvasSize(_width, height);
		for (pair<int, Renderable> entry : _itemMap) {
			_rayTracer->addHittable(*reinterpret_cast<const shared_ptr<Hittable>*>(&entry.second.meshMod));
		}
		if (_config & USE_BVH) {
			_rayTracer->useBVH();
		} else {
			_rayTracer->useHittableList();
		}
		_rayTracer->build();
	}

	void setRenderingMethod(RenderingMethod method) {
		_method = method;
	}

	void setRendererConfig(size_t config) {
		_config = config;
		/*
		switch (method) {
		case RASTERIZATION:
			_renderMethod = &Renderer::rasterization_method;
			_withShadow = false;
			break;
		case RASTERIZATION_WITH_SHADOW:
			_renderMethod = &Renderer::rasterization_method;
			_renderShadowMethod = &Renderer::shadow_map_rendering_method;
			_withShadow = true;
			break;
		case MONTE_CARLO_RAY_TRACING:
			_renderMethod = &Renderer::monte_carlo_ray_tracing_method;
			break;
		}*/

	}

	void rasterization_method() {
		int modelCnt = modelCount();
		std::clog << "Rendering _hittables...\n";
		for (const pair<const int, Renderable> entry : _itemMap) {
			const Renderable& item = entry.second;
			const shared_ptr<MeshModel> meshMod = *reinterpret_cast<const shared_ptr<MeshModel>*>(&item.meshMod);
			Shader::uni.modelView =  _view * (*item.modeling);
			Shader::uni.modelViewIT = Shader::uni.modelView.inverse().transpose();
			Shader::uni.projection = _projection;
			Shader::uni.material = item.material;
			Shader::uni.lights = _lights;
			Shader::uni.ambientLightIntensity = { 10, 10, 10 };
			Shader::uni.withShadow = _withShadow;
			Shader::uni.viewInverse = _view.inverse();
			Shader::uni.objWorld2ShadowMap = _objWorld2ShadowMap;
			Shader::uni.objWorld2LightViewspace = _objWorld2LightViewspace;
			int triCnt = meshMod->meshList.size();
			int remainCnt = triCnt - 1;
			modelCnt--;
			//std::clog << shader->uni.model << "\n\n" << shader->uni.view << "\n\n" << shader->uni.projection << "\n\n" << _viewport << "\n";
		#pragma omp parallel for
			for (int i = 0; i < triCnt; i++) {
				const shared_ptr<Triangle> t = meshMod->meshList[i];
				const shared_ptr<Shader> shader = item.shader->clone();
			#pragma omp critical
				{
					std::clog << "\rModels remaining: " << std::setw(5) << modelCnt << " Current model meshes remaining: " << std::setw(5) << remainCnt << std::flush;
				}
				shader->attr.tangent = t->tangents[0];
				Vec4 screenCoords[3];
				for (int i = 0; i < 3; i++) {
					//for each vertex: Vertex Shader -> Homogeneous divide -> Viewport transform
					//space transform: object space --MVP--> clipping space --divide by w--> NDC --viewport trans--> screen space
					Vec4 clipCoord;
					Vec3 vertexCoord = toVec3(t->vertices[i]);
					Vec3 normal = toVec3(t->normals[i]);
					shader->attr.vertexCoord = vertexCoord;
					shader->attr.normal = normal;
					shader->attr.textureCoord = t->texCoords[i];
					shader->shadeVertex(i, clipCoord);
					//裁剪操作暂时忽略
					Vec4 ndcCoord = clipCoord / clipCoord.w();
					screenCoords[i] = _viewport * ndcCoord;
				}
				_rasterizer->rasterize_triangle(screenCoords, shader);
			#pragma omp atomic
				remainCnt--;
			}
		}
		std::clog << "\n";
		std::clog << "Drawing..." << std::flush;
		_rasterizer->drawOnCanvas(_canvas);
	}

	void shadow_map_rendering_method() {
		int i = 0;
		for (const Light& light : _lights) {
			const ShadowMap& map = light.shadowMap;
			_rasterizer->useExternalBuffer(map.buffer);
			_rasterizer->setSSAAMultiple(1); //对shadowMap不使用SSAA
			_rasterizer->setFrameSize(map.width, map.height);
			_rasterizer->clearBuffer();
			std::clog << "Rendering shadow map...Light "<< i <<'\n';
			_view = getViewing(light.position, light.direction, light.up);
			double h = light.width / light.aspectRatio;
			_projection = getOrthoProjection(-light.maxDistance, -0.1, -light.width / 2, light.width / 2, h / 2, -h / 2); //正交变换,无需透视除法
			//Vec4 v(light.width / 2, light.width / 2, light.width / 2,1);
			//v = _projection * v;
			_viewport = getViewport(map.width, map.height, light.maxDistance,0);
			_objWorld2ShadowMap.push_back(_viewport * _projection * _view);
			_objWorld2LightViewspace.push_back(_view);
			Shader::uni.maxDistance = light.maxDistance;
			_rasterizeShadowMap();
			
			_canvas->setCanvasSize(map.width, map.height);
			_canvas->clearBuffer();
			_rasterizer->drawOnCanvas(_canvas);
			std::ofstream ofile("shadow_map_light_" + std::to_string(i) + ".ppm", std::ios::out);
			_canvas->outputPixelBuffer(ofile);
			ofile.close();
			i++;
		}
	}



	void monte_carlo_ray_tracing_method() {
		int height = _width / _aspectRatio;
		std::clog << "Rendering _hittables...SPP: "<<_samplesPerPixel<<"\n";
		int linecnt = height;
		double scale = tan(deg2rad(40 * 0.5));
	#pragma omp parallel for
		for (int y = 0; y < height; y++) {
		#pragma omp critical
			{
				std::clog << "\rLines remaining: " << std::setw(5) << linecnt-- << std::flush;
			}
		
			for (int x = 0; x < _width; x++) {
				for (int k = 0; k < _samplesPerPixel; k++) {
					Ray ray = _rayTracer->getRay2(Point2i(x, y));
					HitRecord nearestPoint = _rayTracer->trace(ray, Interval(0.001, infinity));
					ColorN c(0,0,0);
					if (nearestPoint.hitAnything) {
						ColorN subc = _rayTracer->shade(nearestPoint, -ray.direction) / _samplesPerPixel;
						Interval(0, 1).clamp(subc);
						c += subc;
					} else {
						c += ColorN(0, 0, 0);
					}
					size_t offset = y * _width + x;
					_canvas->drawOnePixel(c, offset);
				}
			}
		}

	}

	void render() {
		switch (_method) {
		case RASTERIZATION:
			if (_config & WITH_SHADOW) {
				_withShadow = true;
				shadow_map_rendering_method();
			}
			rasterizerConfigs();
			rasterization_method();
			break;
		case MONTE_CARLO_RAY_TRACING:
			rayTracerConfigs();
			monte_carlo_ray_tracing_method();
			break;
		}
	}

	void clearBuffer() {
		_rasterizer->clearBuffer();
		_canvas->clearBuffer();
	}

	void output(std::ostream& out) {
		_canvas->outputPixelBuffer(out);
	}

};

#endif // DEVICE_RENDERER_H
