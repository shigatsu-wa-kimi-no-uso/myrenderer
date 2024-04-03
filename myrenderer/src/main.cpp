#include <common/global.h>
#include <device/Renderer.h>
#include <device/OBJLoader.h>
#include <hittable/MeshModel.h>
#include <common/utility.h>
#include <device/shader/NormalShader.h>
#include <device/shader/TextureShader.h>
#include <texture/ImageTexture.h>

void setCamera(Camera& camera) {
	camera.aspect_ratio = 1;
	camera.fov = 45;
	camera.frustum_far = -50;	//基于lookat的远近值
	camera.frustum_near = -0.1;
	camera.lookfrom = Point3(0, 0, 10);
	camera.lookat = Vec3(0, 0, -1);
	camera.vup = Vec3(0, 1, 0);
}


int main() {
	
	string objPath = "C:/Users/Administrator/Documents/Visual Studio 2022/Projects/myrenderer/myrenderer/models/spot/spot_triangulated_good.obj";
	string texturePath = "C:/Users/Administrator/Documents/Visual Studio 2022/Projects/myrenderer/myrenderer/models/spot/spot_texture.png";
	shared_ptr<MeshModel> model = OBJLoader::loadMeshModel(objPath);

	Renderer renderer;
	Camera camera;
	setCamera(camera);

	//assets registration
	shared_ptr<Texture> texture = make_shared<ImageTexture>(texturePath);
	int normalShaderId = renderer.addShader(make_shared<NormalShader>());
	int textureShaderId = renderer.addShader(make_shared<TextureShader>());
	int voidMaterialId = renderer.addMaterial(make_shared<Material>());
	int blinnPhong1Id = renderer.addMaterial(make_shared<BlinnPhong>(ColorN(0.005, 0.005, 0.005),ColorN(0.7937, 0.7937, 0.7937),150, texture));

	renderer.addLight(Light{ { 20, 20, 20 }, { 500, 500, 500 } });
	renderer.addLight(Light{ {-20, 20, 0}, {500, 500, 500} });

	renderer.setCanvasWidth(700);
	renderer.setCanvasAspectRatio(1);

	int idx = renderer.addMeshModel(model);

	renderer.bindModelProperties(idx, textureShaderId, blinnPhong1Id);
	Mat4 modeling = getModeling(Point3(0, 0, 0), Vec3(2.5, 2.5, 2.5), Vec3(0, 1, 0), 140);
	renderer.setModelTranslation(idx, make_shared<Mat4>(modeling));

	int idx2 = renderer.addMeshModel(model);
	renderer.bindModelProperties(idx2, textureShaderId, blinnPhong1Id);

	Mat4 modeling2 = getModeling(Point3(1, 1, -2), Vec3(1, 1, 1), Vec3(0, 1, 0), 0);
	renderer.setModelTranslation(idx2, make_shared<Mat4>(modeling2));

	renderer.setCamera(make_shared<Camera>(camera));
	renderer.setRenderMethod(Renderer::RASTERIZATION);
	renderer.setSSAAMultiple(4);
	renderer.clearBuffer();
	renderer.render();
	renderer.output(std::cout);

}
