#include <common/global.h>
#include <device/Renderer.h>
#include <device/OBJLoader.h>
#include <hittable/MeshModel.h>
#include <common/utility.h>
#include <device/shader/NormalShader.h>
#include <device/shader/TextureShader.h>
#include <device/shader/CheckerTextureShader.h>
#include <texture/ImageTexture.h>
#include <material/BlinnPhongWithBump.h>
#include <device/shader/BumpTextureShader.h>
#include <texture/CheckerTexture.h>


void setCamera(Camera& camera) {
	camera.aspect_ratio = 1;
	camera.fov = 45;
	camera.frustum_far = -50;	//基于lookat的远近值
	camera.frustum_near = -0.1;
	camera.lookfrom = Point3(0, 1, 5);
	camera.lookat = Vec3(0, -0.5, -1);
	camera.vup = Vec3(0, 1, 0);
}


shared_ptr<MeshModel> getParallelogram(const Vec3& u,const Vec3& v,const Point3& q) {
	shared_ptr<MeshModel> mod = make_shared<MeshModel>();
	Point3 a = q + u;
	Point3 b = q + v;
	Point3 c = q + u + v;
	Triangle t1;
	t1.vertices[0] = toVec4(q,1);  //q
	t1.vertices[1] = toVec4(a, 1); //a
	t1.vertices[2] = toVec4(b, 1); //b
	t1.normals[0] = toVec4((b - q).cross(a - q), 0);
	t1.normals[1] = toVec4((q - a).cross(b - a), 0);
	t1.normals[2] = toVec4((q - b).cross(a - b), 0);
	Triangle t2 = t1;
	t2.vertices[0] = toVec4(c,1);
	t2.normals[2] = toVec4((c - a).cross(c - b), 0);
	mod->meshList.push_back(t1);
	mod->meshList.push_back(t2);
	return mod;
}



std::unordered_map<string, int> shaders;
std::unordered_map<string, int> materials;
std::unordered_map<string, shared_ptr<Texture>> textures;




void registerAssets(Renderer& renderer) {
	string texturePath = "C:/Users/Administrator/Documents/Visual Studio 2022/Projects/myrenderer/myrenderer/models/spot/spot_texture.png";
	string bumpTexturePath = "C:/Users/Administrator/Documents/Visual Studio 2022/Projects/myrenderer/myrenderer/models/spot/hmap.jpg";

	shaders["bump"] = renderer.addShader(make_shared<BumpTextureShader>());
	shaders["normal"] = renderer.addShader(make_shared<NormalShader>());
	shaders["texture"] = renderer.addShader(make_shared<TextureShader>());
	shaders["checker"] = renderer.addShader(make_shared<CheckerTextureShader>());

	textures["spot"] = make_shared<ImageTexture>(texturePath);
	textures["bump_spot"] = make_shared<ImageTexture>(bumpTexturePath);
	textures["checker"] = make_shared<CheckerTexture>(Color255(255, 174, 201), Color255(255, 127, 39));

	materials["checker"] = renderer.addMaterial(make_shared<Lambertian>(textures["checker"]));
	shared_ptr<BlinnPhong> blinnPhong = make_shared<BlinnPhong>(ColorN(0.005, 0.005, 0.005), ColorN(0.7937, 0.7937, 0.7937), 150, textures["spot"]);
	materials["blinn_phong_spot"] = renderer.addMaterial(blinnPhong);
	materials["blinn_phong_with_bump_spot"] = renderer.addMaterial(make_shared<BlinnPhongWithBump>(*blinnPhong, textures["bump_spot"]));

}


void addSpot(Renderer& renderer) {
	string objPath = "C:/Users/Administrator/Documents/Visual Studio 2022/Projects/myrenderer/myrenderer/models/spot/spot_triangulated_good.obj";
	shared_ptr<MeshModel> model = OBJLoader::loadMeshModel(objPath);

	int spot_id = renderer.addMeshModel(model);

	renderer.bindModelProperties(spot_id, shaders["texture"], materials["blinn_phong_spot"]);
	Mat4 modeling = getModeling(Point3(0, 0, 0), Vec3(1, 1, 1), Vec3(0, 1, 0), 140);
	renderer.setModelTransform(spot_id, make_shared<Mat4>(modeling));
}

void addChecker(Renderer& renderer) {
	shared_ptr<MeshModel> checker = getParallelogram(Vec3(5, 0, 0), Vec3(0, 0, -5), Point3(0, 0, 0));
	int checker_id = renderer.addMeshModel(checker);
	Mat4 modeling2 = getModeling(Point3(-3, -1, 0), Vec3(1, 1, 1), Vec3(0, 1, 0), 0);
	renderer.bindModelProperties(checker_id, shaders["checker"],materials["checker"]);
	renderer.setModelTransform(checker_id, make_shared<Mat4>(modeling2));
}


int main() {
	
	Renderer renderer;
	Camera camera;
	setCamera(camera);
	renderer.setCamera(make_shared<Camera>(camera));

	renderer.addLight(Light{ { 20, 20, 20 }, { 500, 500, 500 } });
	renderer.addLight(Light{ {-20, 20, 0}, {500, 500, 500} });

	renderer.setCanvasWidth(700);
	renderer.setCanvasAspectRatio(1);

	registerAssets(renderer);
	addChecker(renderer);
	addSpot(renderer);


	renderer.setRenderMethod(Renderer::RASTERIZATION);
	renderer.setSSAAMultiple(4);
	renderer.applyConfigs();
	renderer.clearBuffer();

	renderer.render();
	renderer.output(std::cout);

}
