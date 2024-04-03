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

class Renderer {
private:
	struct Renderable {
		shared_ptr<MeshModel> meshMod;
		shared_ptr<Mat4> modeling;
		shared_ptr<Material> material;	//ģ������� shader֮���Ƕ�Զ��ϵ
		shared_ptr<Shader> shader;
	};
	std::vector<Light> globalLights;
	using ItemMap = std::unordered_map<int, Renderable>;
	shared_ptr<Camera> _camera; 
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
	int    _width;  // �����ͼ��,����������,�����ӿڵĿ�ȣ�
	int    _samplesPerPixel;   // ���ͼ��ÿ���ز�����
	

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
		// Զ����[-1,1]�Ĺ�ϵ: zFar --> -1 zNear --> 1 
		Mat4 projection;
		double f = _camera->frustum_far, n = _camera->frustum_near;			// ���� camera ��camera����ϵ������ -z ��, ԽԶԽС
		double t = std::tan(pi * (_camera->fov / 2.0) / 180.0) * std::abs(n);
		double b = -t, r = t * _camera->aspect_ratio, l = -r;  //������������ϵ��b,t,l,r  ��canvas�Ĵ�С�����ͼ��Ĵ�С����һ����
		// camera��aspect_ratio�����ͼ���aspect ratio���ܲ���ͬ
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
		// ��׶��->�ü��ռ�				// [n,f] -> [-1*w,1*w]
		ortho = scale * trans;
		//��׼������->͸��ת����
		projection = ortho * persp2ortho;
		_projection = projection;
	}

	void _updateViewportMatrix(){
		//ע��: �����z�����,����ܸı�z��ֵ�Ĵ�С������Զ���Ĺ�ϵ
		//�˱任��, ndc_z = -1 --> screen_z=zNear, ndc_z = 1 --> screen_z=zFar,  zFar < zNear < 0
		//����: NDC([-1,1]^3)   near = -1, far = 1
		double zFar = _camera->frustum_far, zNear = _camera->frustum_near; 
		double f1 = abs(zFar - zNear) / 2.0; // zLen/2.0
		double f2 = abs(zFar + zNear) / 2.0; // �о���ֵ��
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
		t2 <<						//��
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, f2,			// [-zLen/2,zLen/2] -> [min(abs(zF,zN)) (far) ,max(abs(zF,zN)) (near)] (��ֵ,ԽСԽ��)
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

	using MaterialMap = std::unordered_map<int, shared_ptr<Material>>;
	using ShaderMap = std::unordered_map<int, shared_ptr<Shader>>;
	using TextureMap = std::unordered_map<int, shared_ptr<Texture>>;

	MaterialMap _materialMap;
	ShaderMap _shaderMap;
	TextureMap _textureMap;

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



	void setSSAAMultiple(int multiple) {
		_rasterizer->setSSAAMultiple(multiple);
	}

	int addMeshModel(const shared_ptr<MeshModel>& meshMod) {
		_itemMap[++_lastIndex] = { meshMod,make_shared<Mat4>(_defaultModeling)};
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
		globalLights.push_back(light);
	}

	shared_ptr<Texture> getTexture(int textureId) {
		return _textureMap[textureId];
	}

	void bindModelProperties(int renderableId, int shaderId,int materialId) {
		setModelMaterial(renderableId, materialId);
		setModelShader(renderableId, shaderId);
	}

	void setModelTranslation(int meshModIdx,const shared_ptr<Mat4>& mt) {
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
			const shared_ptr<Shader>& shader = item.shader;
			shader->uni.model = *item.modeling;
			shader->uni.view = _view;
			shader->uni.projection = _projection;
			shader->uni.material = item.material;
			shader->uni.eyePos = _camera->lookfrom;
			shader->uni.lights = globalLights;
			shader->uni.ambientLightIntensity = { 10, 10, 10 };
			int triCnt = item.meshMod->meshList.size();
			modelCnt--;
			//std::clog << shader->uni.model << "\n\n" << shader->uni.view << "\n\n" << shader->uni.projection << "\n\n" << _viewport << "\n";
			for (const Triangle& t : item.meshMod->meshList) {
				triCnt--;
				std::clog << "\rModels remaining: " << std::setw(5)  << modelCnt<<" Current model meshes remaining: " <<std::setw(5) << triCnt << std::flush;
				Vec4 screenCoords[3];
				for (int i = 0; i < 3; i++) {
					//for each vertex: Vertex Shader -> Homogeneous divide -> Viewport transform
					//space transform: object space --MVP--> clipping space --divide by w--> NDC --viewport trans--> screen space
					Vec4 clipCoord;
					Vec3 vertexCoord = toVec3(t.vertices[i]);
					Vec3 normal = toVec3(t.normals[i]);
					shader->attr.vertexCoord = vertexCoord;
					shader->attr.normal = normal;
					shader->attr.textureCoord = t.texCoords[i];
					shader->shadeVertex(i, clipCoord);
					//�ü�������ʱ����
					Vec4 ndcCoord = clipCoord / clipCoord.w();
					screenCoords[i] = _viewport * ndcCoord;

					//debug
					/*
					static int cnt = 0;
					std::cout << cnt++ << " " << i << " " << screenCoords[i][0] << " " << screenCoords[i][1] << " " << screenCoords[i][2] << "\n";*/
				}
				_rasterizer->rasterize_triangle(screenCoords,shader);
			
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

#endif // DEVICE_RENDERER_H
